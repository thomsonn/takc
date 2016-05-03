#ifndef _TREE_H_
#define _TREE_H_

#include "move.h"

typedef struct tree_s {
    node_s *first;
    int free;
} tree_s;

typedef struct node_s {
    node_s *parent;
    node_s *sibling;
    node_s *child;
    move_s move;
    /* TODO: Should be able to infer player */
    int player;
    double wins;
    double total;
} node_s;

node_s *tree_append(node_s *, move_s);
int tree_n_children(node_s *);
node_s *tree_nth_child(node_s *, int);
void tree_unlink(node_s *);

#endif /*_TREE_H_ */
