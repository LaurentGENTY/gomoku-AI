/**
 * @file player3.c
 * Player evaluate the next best move and plays it if possible. He has 3 matrices for opportunities (potentials moves that create lines for him), danger (potentials moves that create lines for the opponent) and the current board. He computes the best moves for him and for his opponent and if the opponent has better chance to make lines then he plays to counter the opponent; otherwise, he plays for lenghtening his lines. If there are several possibilities with the same strengh, he chooses one of them randomly
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../server/move.h"
#include "player.h"
#include "matrix.h"
#include "heuristic0.h"

/* Struct for containing data by the player */
struct player me;

/* External functions */

char const* get_player_name(){
  return "C warrior 3.0";
}

struct col_move_t* propose_opening(size_t size){
  struct col_move_t * ret = malloc(sizeof(*ret) * 3);
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
  assert(opening);
  /* it will always return 0 because the black has one more move */
  struct matrix * opening_1 = matrix_generate(size, 0);
  struct matrix * opening_2 = matrix_generate(size, 0);
  for(int i = 0; i < 3; i++){
    matrix_set(opening_1, opening[i].m.row, opening[i].m.col, -1);
    matrix_set(opening_2, opening[i].m.row, opening[i].m.col, -1);
  }

  update_potential_lines(opening[0].m.row, opening[0].m.col, opening_1, self.color);
  update_potential_lines(opening[1].m.row, opening[1].m.col, opening_2, (self.color+1)%2);
  update_potential_lines(opening[2].m.row, opening[2].m.col, opening_1, self.color);

  int a = matrix_max(opening_1);
  int b = matrix_max(opening_2);

  matrix_free(opening_1);
  matrix_free(opening_2);
  return b > a;
}

void initialize(size_t size, enum color_t id){
  self.color = id;
  self.size = size;
  self.opportunity = matrix_generate(size, 0);
  self.danger = matrix_generate(size, 0);
  self.actual_board = matrix_generate(size, -1);
  time_t t;
  srand((unsigned) time(&t));
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  /* Actualize the matrix with previous_moves */
  for(size_t i = 0; i < n_moves; i++){
    heuristic_actualize(previous_moves[i].m, previous_moves[i].c);
  }
  /* Extract the list of max for both of them */
  struct list * list_moves = generating_moves(self.opportunity, self.danger);
  /* Choose randomly a point */
  int alea = rand() % list_length(list_moves);
  struct move_t mv = list_get(list_moves, alea);
  list_free(list_moves);
  return mv;
}

void finalize(){
  matrix_free(self.opportunity);
  matrix_free(self.danger);
  matrix_free(self.actual_board);
}
