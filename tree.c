#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "fastlog.h"
#include "tree.h"

tree_s *tree_new(int size)
{
    tree_s *tree;
    node_s *root;

    tree = malloc(sizeof(tree_s));
    root = malloc(size*sizeof(node_s));

    tree->first = root;
    tree->root = root;
    tree->free = NULL;

    root->parent = NULL;
    root->sibling = NULL;
    root->child = NULL;
    root->refcount = 1;
    root->wins = 0;
    root->total = 0;

    for (int i = size; i >= 0; --i) {
	node_s *node;

	node = &tree->first[i];
	node->refcount = 0;
	node->sibling = tree->free;
	tree->free = node;
    }

    return tree;
}

void tree_destroy(tree_s *tree, node_s *node)
{
    node_s *parent = node->parent;

    node->refcount--;
    if (node == parent->child)
	parent->child = node->sibling;
    else {
	node_s *prev;

	for (prev = parent->child; prev; prev = prev->sibling)
	    if (prev->sibling == node)
		break;

	prev->sibling = node->sibling;
    }

    while (node->refcount--) {
	node_s *next;

	if (node->child)
	    next = node->child;
	else if (node->sibling)
	    next = node->sibling;
	else
	    next = node->parent;

	if (node->refcount == 0) {
	    /* Prepend to the free list */
	    node->sibling = tree->free;
	    tree->free = node;
	}

	node = next;
    }
}

static node_s *select_ucb1_worst(node_s *node)
{
    node_s *worst = node->child;
    double min = INFINITY;
    double log_total = fastlog2(node->total);

    for (node_s *child = node->child; child; child = child->sibling) {
	double eval = child->wins/child->total + 1.7*sqrt(log_total/child->total);

	if (eval < min) {
	    min = eval;
	    worst = child;
	}
    }

    return worst;
}

node_s *tree_prepend(tree_s *tree, node_s *node, move_s move)
{
    node_s *new;

    if (!tree->free)
	/* FIXME: Maintain some minimum breadth at each level,
	   need to do a backtracking search on worst branches */
	tree_destroy(tree, select_ucb1_worst(tree->root));

    /* Failure is not an option */
    assert(tree->free);

    /* Create a new child */
    new = tree->free;
    tree->free = new->sibling;

    new->parent = node;
    new->sibling = node->child;
    new->child = NULL;
    new->refcount = 1;
    new->move = move;
    new->wins = 0;
    new->total = 0;

    /* Insert the child beneath the node */
    if (node->child == NULL)
	node->refcount++;
    node->child = new;

    return new;
}

int tree_n_children(node_s *node)
{
    node_s *child;
    int n = 0;

    for (child = node->child; child; child = child->sibling)
	n++;

    return n;
}

node_s *tree_nth_child(node_s *node, int i)
{
    node_s *child = node->child;

    for (; i > 0; --i)
	child = child->sibling;

    return child;
}

void tree_unlink(tree_s *tree, node_s *node)
{
    node_s *parent = node->parent;

    for (node_s *child = parent->child; child; child = child->sibling) {
	if (child == node)
	    continue;

	tree_destroy(tree, child);
    }

    if (--parent->refcount == 0) {
 	/* Prepend to the free list */
	parent->sibling = tree->free;
	tree->free = parent;
    }

    node->parent = NULL;
}

#ifdef TEST_TREE

#include <glib.h>

#define TEST_SIZE 100

typedef struct tree_fixture_s {
    tree_s *tree;
} tree_fixture_s;

static void tree_setup(tree_fixture_s *fixture, gconstpointer test_data)
{
    fixture->tree = tree_new(TEST_SIZE);
}

static void tree_teardown(tree_fixture_s *fixture, gconstpointer test_data)
{
    free(fixture->tree->first);
    free(fixture->tree);
}

static void test_new(tree_fixture_s *fixture, gconstpointer test_data)
{
    tree_s *tree = fixture->tree;

    for (int i = 0; i < TEST_SIZE; i++) {
	node_s *node;

	node = &tree->first[i];
	g_assert(node->refcount == 0);
    }
}

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add("/tree/new", tree_fixture_s, NULL, tree_setup, test_new, tree_teardown);

    return g_test_run();
}

#endif
