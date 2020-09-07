/**
 * @file player4.2.c
 * This player is quite similar is player4.c but it uses a different method to do minmax: with the number of nodes and not the depth. This player will deepen the possibilities by using min-max strategy. When he has several possibilities, he explores all of them with min_max to find the best option in the long term
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "player.h"
#include "../server/move.h"
#include "matrix.h"
#include "heuristic.h"

/* Variable that defines a 100% winning move */
#define INFINITY 10000

/**
 * @brief Struct for containing data by the player
 */
struct player{
  /**
   * own heuristic to evaluate board
   */
  struct heuristic * h;
  /**
   * size of the board
   */
  size_t size;
  /**
   * id of the player
   */
  int color;
  /**
   * current number of moves
   */
  int nb_moves;
};

/* Struct for containing data by the player */
struct player self;

/* Internal functions */

/**
 * @brief Min-max strategy that deepens in tree of possibilities of a number of nodes given, computes the evaluation of the board and find the best move to do by taking into account both matrix of opportunities (player) and dangers (opponent). Each recursive call reduce the number of nodes left and when there is nodes left, computes the evaluation of the board.
 * @param[in] nb_nodes number of nodes left to explore
 * @param[in] alpha minimum value previously found in other branches
 * @param[in] beta maximum value previously found in other branches
 * @param[in] maximizingPlayer id of the current player
 * @return evaluation of the board at a given depth
 */
int min_max(int * nb_nodes, int alpha, int beta, int maximizingPlayer){
  *nb_nodes = *nb_nodes - 1;
  if (*nb_nodes <= 0 || heuristic_full(self.h)){
    return heuristic_eval_game(self.h, self.color);
  }
  int better_move;
  int step;
  if (maximizingPlayer){
    better_move = -INFINITY;
    struct list * moves = generating_moves(self.h, self.color);
    if (heuristic_potential_point(self.h, list_get(moves, 0)) >= 5){
      list_free(moves);
      return INFINITY-1;
    }
    for(int i = 0; i < list_length(moves); i++){
      struct move_t mv = list_get(moves, i);
      heuristic_actualize(self.h, mv, self.color); // simulate the move
      step = min_max(nb_nodes, alpha, beta, 0);
      heuristic_remove(self.h, mv); //undo what was done
      if (step > better_move){ better_move = step; }
      if (step > alpha){ alpha = step; }
      if (beta <= alpha){ break; }
    }
    list_free(moves);
  }
  else{
    better_move = INFINITY;
    struct list * moves = generating_moves(self.h, (self.color+1)%2);
    if (heuristic_potential_point(self.h, list_get(moves, 0)) >= 5){
      list_free(moves);
      return -INFINITY+1;
    }
    for(int i = 0; i < list_length(moves); i++){
      struct move_t mv = list_get(moves, i);
      heuristic_actualize(self.h, mv, (self.color+1)%2);
      step = min_max(nb_nodes, alpha, beta, 1);
      heuristic_remove(self.h, mv);
      if (step < better_move){ better_move = step; }
      if (step < beta){ beta = step; }
      if (beta <= alpha){ break; }
    }
    list_free(moves);
  }
  return better_move;
}

/* External functions */

char const* get_player_name(){
  return "C warrior 4.2";
}

struct col_move_t* propose_opening(size_t size){
  struct col_move_t cm;
  initialize(size,BLACK);
  cm.m.row = size / 2;
  cm.m.col =  size / 2;
  cm.c = self.color;
  struct col_move_t * cmv = malloc(sizeof(struct col_move_t)*3);
  *cmv = cm;
  struct col_move_t cm1;
  struct col_move_t cm2;
  self.color = WHITE;
  cm1.m = play(cmv,1);
  cm1.c = self.color;
  *(cmv+1) = cm1;
  self.color = BLACK;
  cm2.m = play(cmv,2);
  cm2.c = BLACK;
  *(cmv+2) = cm2;
  self.nb_moves = 0;
  finalize();
  return cmv;
}

int accept_opening(size_t size, const struct col_move_t* opening){
  initialize(size,WHITE);
  struct move_t m = play(opening,3);
  heuristic_actualize(self.h, m, self.color);
  int h = heuristic_eval_game(self.h, self.color);
  struct col_move_t cmv1 = {m,WHITE};
  struct col_move_t* open = malloc(sizeof(struct col_move_t)*4);
  *open = *opening ;
  *(open + 1) = *(opening + 1);
  *(open + 2) = *(opening + 2);
  *(open + 3) = cmv1;
  self.color = BLACK;
  struct move_t m1 = play(open,4);
  heuristic_actualize(self.h, m1, self.color);
  int h1 = heuristic_eval_game(self.h, self.color);
  heuristic_remove(self.h,m);
  heuristic_remove(self.h,m1);
  self.nb_moves = 0;
  free(open);
  finalize();
  if(h1 < h){
    return 0;
  }
  return 1;
}

void initialize(size_t size, enum color_t id){
  self.color = id;
  self.size = size;
  self.nb_moves = 0;
  self.h = heuristic_init(size, id);
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  // updating actual board
  for(size_t i = 0; i < n_moves; i++){
    heuristic_actualize(self.h, previous_moves[i].m, previous_moves[i].c);
    self.nb_moves++;
  }

  if (self.nb_moves == 0){
    struct move_t mv = {self.size/2, self.size/2};
    return mv;
  }
  /*printf("===============================\n");
    matrix_print(&self.h->opportunity);
    printf("===============================\n");
    matrix_print(&self.h->danger);
    printf("===============================\n");*/
  // applying min_max
  struct move_t next_mv;

  int nb_nodes = 100000; // number of nodes to explore

  int better_move = -INFINITY;
  int alpha = -INFINITY;
  int beta = INFINITY;
  int step;
  struct list * moves = generating_moves(self.h, self.color);
  struct move_t m = list_get(moves, 0);
  if (heuristic_potential_point(self.h, m) >= 5){
    list_free(moves);
    return m;
  }
  if (list_length(moves) == 0){printf("error !\n");}
  for(int i = 0; i < list_length(moves); i++){
    struct move_t mv = list_get(moves, i);
    //printf("move %ld %ld value is %d\n", mv.row, mv.col, heuristic_potential_point(self.h, m));
    heuristic_actualize(self.h, mv, self.color);
    step = min_max(&nb_nodes, alpha, beta, 0);
    if (step > better_move){
      better_move = step;
      next_mv = mv;
    }
    if (step > alpha){ alpha = step; }
    if (beta <= alpha){ break; }
    heuristic_remove(self.h, mv);
  }
  list_free(moves);
  printf("final node %d\n", nb_nodes);
  return next_mv;
}

void finalize(){
  heuristic_free(self.h);
}
