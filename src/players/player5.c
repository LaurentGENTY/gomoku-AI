/**
 * @file player5.c
 * Player that uses a Montecarlo strategy find at https://fr.wikipedia.org/wiki/Recherche_arborescente_Monte-Carlo (not working though)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "player.h"
#include "../server/move.h"
#include "matrix.h"
#include "montecarlo.h"
#include "../server/board.h"

/* Struct for containing data by the player */
struct player self;

/* External functions */

char const* get_player_name(){
  return "C warrior 5.1";
}

struct col_move_t* propose_opening(size_t size){
    struct col_move_t * ret = malloc(sizeof(*ret) * 3);
    //DO NOT FORGET TO FREE BY SERVER
    time_t t;
    srand((unsigned) time(&t));
    ret[0].m.row = rand() % size;
    ret[0].m.col = rand() % size;
    ret[1].m.row = rand() % size;
    ret[1].m.col = rand() % size;
    while (ret[1].m.col == ret[0].m.col){ret[1].m.col = rand() % size;}
    ret[2].m.row = rand() % size;
    ret[2].m.col = ret[1].m.col;
    while (ret[2].m.row == ret[1].m.row ){ret[2].m.row = rand() % size;}

    ret[0].c = BLACK;
    ret[1].c = WHITE;
    ret[2].c = BLACK;
    return ret;
}

int accept_opening(size_t size, const struct col_move_t* opening){
     return 0; // j'ai pas confiance...
}

void initialize(size_t size, enum color_t id){
    self.tree = generate_tree();
    self.color = id;
    self.size = size;
    self.nb_moves = 0;
    self.board = board__initialize(size);
    time_t t;
    srand((unsigned) time(&t));
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
    // updating actual board
    // if possible, move in the tree
    for(size_t i = 0; i < n_moves; i++){
      board__add_move(self.board, previous_moves[i].m, previous_moves[i].c);
      self.nb_moves++;
      relocate_root(&self.tree, previous_moves[i]);
    }
    //self.tree = generate_tree();
    // simulate
    for(int i = 0; i < 300; i++){
        //printf("simulation %d\n",i);
        struct node * n = selection(&self.tree, self.color);
        expansion(&self.tree, n);
    }
    struct move_t mv = get_better_move(&self.tree);
    //free_tree(&self.tree);
    return mv;
}

void finalize(){
    board__free(self.board);
    free_tree(&self.tree);
}
