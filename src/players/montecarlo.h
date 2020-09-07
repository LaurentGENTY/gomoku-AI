#ifndef MONTE_H
#define MONTE_H

#include "../server/move.h"
#include "matrix.h"

struct node{
    double nb_simulations;
    double nb_wins;
    struct node ** childs;
    int nb_childs;
    struct node * father;
    int game_over;
    struct move_t mv;
};
struct tree{
    struct node * root;
    struct board * tmp;      // board to save move during selection
    int color_for_expansion;
    int nb_moves;
};

struct player{
    size_t size;
    int color;
    int nb_moves;
    struct tree tree;
    struct board * board;
};

void free_node(struct node * n);
void free_tree(struct tree * t);
struct node * generate_node(struct node * father, struct move_t mv);
struct tree generate_tree();
struct move_t get_better_move(struct tree * t);
void relocate_root(struct tree *t, struct col_move_t mv);

// from the root, find a leaf
// exploration vs exploitation
struct node * selection(struct tree * t, int color);

// add child to the tree if it is not the end
void expansion(struct tree * t, struct node * n);

// simulate a game until victory randomly
// return 1 the color of the winner
int simulation(struct tree * t, struct node * n);

// update all ancestors of the leaf simulated with the final score
void update(struct node * n, int simulated_res);


#endif
