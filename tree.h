#ifndef _TREE_H_
#define _TREE_H_

#include "move.h"

typedef struct node_s {
    struct node_s *parent;
    struct node_s *sibling;
    struct node_s *child;
    int refcount;
    move_s move;
    /* TODO: Should be able to infer player */
    int player;
    double wins;
    double total;
} node_s;

typedef struct tree_s {
    node_s *first;
    node_s *root;
    node_s *free;
} tree_s;

tree_s *tree_new(int);
void tree_destroy(tree_s *, node_s *);
node_s *tree_prepend(tree_s *, node_s *, move_s);
int tree_n_children(node_s *);
node_s *tree_nth_child(node_s *, int);
void tree_unlink(tree_s *, node_s *);

#endif /*_TREE_H_ */
