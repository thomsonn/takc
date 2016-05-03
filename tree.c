#include "tree.h"

node_s *tree_append(node_s *node, move_s move)
{
    /* TODO: Release memory if necessary */

    /* TODO: Return pointer to new child */
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

    for (; i > 0; i--)
	child = child->sibling;

    return child;
}

void tree_unlink(node_s *node)
{
    /* TODO: Free all other branches */
    node->parent = NULL;
}

#ifdef TEST_TREE

#define TEST_SIZE 100

struct tree_fixture_s {
    node_s tree;
} tree_fixture_s;

void tree_setup(tree_fixture_s *fixture, gconstpointer test_data)
{
    fixture->tree = malloc(TEST_SIZE*sizeof(node_s));
}

void tree_teardown(tree_fixture_s *fixture, gconstpointer test_data)
{
    free(fixture->tree);
}

/* TODO: Write tests */

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    return g_test_run();
}

#endif
