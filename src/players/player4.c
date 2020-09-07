/**
 * @file player4.c
 * This player is an improvement of player3 using min_max. In fact, the previous player only does the differences between opportunities and dangers at the current turn, but here this player will deepen the possibilities by using min-max strategy. When he has several possibilities, he explores all of them with min_max to find the best option in the long term
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "player.h"
#include "../server/move.h"
#include "matrix.h"
#include "heuristic0.h"

/* Variable that defines a 100% winning move */
#define INFINITY 10000

/* Struct for containing data by the player */
struct player self;
int nb_nodes = 0;

/* Internal functions */

/**
 * @brief Min-max strategy that deepens in tree of possibilities at a depth given, computes the evaluation of the board and find the best move to do by taking into account both matrix of opportunities (player) and dangers (opponent).
 * @param[in] depth depth of the tree of possibilies explored
 * @param[in] alpha minimum value previously found in other branches
 * @param[in] beta maximum value previously found in other branches
 * @param[in] maximizingPlayer id of the current player of the current depth
 * @return evaluation of the board at a given depth
 */
int min_max(int depth, int alpha, int beta, int maximizingPlayer){
    nb_nodes++;
  if (depth == 0 || heuristic_full()){
    return heuristic_eval_game(self.opportunity, self.danger);
  }
  int better_move;
  int step;
  if (maximizingPlayer){
    better_move = -INFINITY;
    struct list * moves = generating_moves(self.opportunity, self.danger);
    struct move_t m = list_get(moves, 0);
    if (matrix_get(self.opportunity, m.row, m.col) >= 5){
      list_free(moves);
      return INFINITY-1;
    }
    for(int i = 0; i < list_length(moves);i++){
      struct move_t mv = list_get(moves, i);
      heuristic_actualize(mv, self.color); /* simulate the move */
      step = min_max(depth-1, alpha, beta, 0);
      heuristic_remove(mv); /* undo what was done */
      if (step > better_move){ better_move = step; }
      if (step > alpha){ alpha = step; }
      if (beta <= alpha){ break; }
    }
    list_free(moves);
  }
  else{
    better_move = INFINITY;
    struct list * moves = generating_moves(self.danger, self.opportunity);
    struct move_t m = list_get(moves, 0);
    if (matrix_get(self.danger, m.row, m.col) >= 5){
      list_free(moves);
      return -INFINITY+1;
    }
    for(int i = 0; i < list_length(moves); i++){
      struct move_t mv = list_get(moves, i);
      heuristic_actualize(mv, (self.color+1)%2);
      step = min_max(depth-1, alpha, beta, 1);
      heuristic_remove(mv);
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
  return "C warrior 4.1";
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

void initialize(size_t size, enum color_t id){
  self.color = id;
  self.size = size;
  self.opportunity = matrix_generate(size, 0);
  self.danger = matrix_generate(size, 0);
  self.actual_board = matrix_generate(size, -1);
}

int accept_opening(size_t size, const struct col_move_t* opening){
  initialize(size,WHITE);
  struct move_t m = play(opening,3);
  heuristic_actualize(m, self.color);
  int h = heuristic_eval_game(self.opportunity, self.danger);
  struct col_move_t cmv1 = {m,WHITE};
  struct col_move_t* open = malloc(sizeof(struct col_move_t)*4);
  *open = *opening ;
  *(open + 1) = *(opening + 1);
  *(open + 2) = *(opening + 2);
  *(open + 3) = cmv1;
  self.color = BLACK;
  struct move_t m1 = play(open,4);
  heuristic_actualize(m1, self.color);
  int h1 = heuristic_eval_game(self.opportunity, self.danger);
  heuristic_remove(m);
  heuristic_remove(m1);
  free(open);
  self.nb_moves = 0;
  finalize();
  if(h1<h){
    return 0;
  }
  return 1;
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  // updating actual board
  for(size_t i = 0; i < n_moves; i++){
    heuristic_actualize(previous_moves[i].m, previous_moves[i].c);
  }

  /*printf("================================\n");
    matrix_print(&self.opportunity);
    printf("================================\n");
    matrix_print(&self.danger);
    printf("================================\n");*/

  // applying min_max
  struct move_t next_mv;
  int depth = 5;
  if (self.nb_moves < 20){ depth = 4; }
  if (self.nb_moves < 10){ depth = 3; }
  if (self.nb_moves < 5){ depth = 2; }

  long clk_tck = CLOCKS_PER_SEC;
  clock_t t1, t2;
  t1 = clock();
  nb_nodes = 0;

  int better_move = -INFINITY;
  int alpha = -INFINITY;
  int beta = INFINITY;
  int step;
  struct list * moves = generating_moves(self.opportunity, self.danger);
  struct move_t m = list_get(moves, 0);
  if (matrix_get(self.opportunity, m.row, m.col) >= 5){
    printf("I won !\n");
    list_free(moves);
    return m;
  }
  if (list_length(moves) == 0){printf("error !\n");}
  for(int i = 0; i < list_length(moves); i++){
    struct move_t mv = list_get(moves, i);
    heuristic_actualize(mv, self.color);
    step = min_max(depth, alpha, beta, 0);
    if (step > better_move){
      better_move = step;
      next_mv = mv;
    }
    if (step > alpha){ alpha = step; }
    if (beta <= alpha){ break; }
    heuristic_remove(mv);
  }
  list_free(moves);
  t2 = clock();
  printf("%d noeuds en %lf secondes\n", nb_nodes, (double)(t2-t1)/(double)clk_tck);
  return next_mv;
}

void finalize(){
  matrix_free(self.opportunity);
  matrix_free(self.danger);
  matrix_free(self.actual_board);
}
