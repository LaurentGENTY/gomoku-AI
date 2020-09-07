/**
 * @file heuristic.c
 * Implementation of heuristic.h for the minmax player (player4.2.so)
 */

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"
#include "list.h"
#include "heuristic.h"
#include "../server/board.h"

/* Internal functions */

int size_line(struct heuristic * h, int i, int j, int di, int dj, int color){
  int row = 1;
  int right = 1;
  int left = 1;

  for(int k = 1; k < 5; k++){
    if ((0 <= i+k*di)
    &&(i+k*di < h->size)
    && (0 <= j+k*dj)
    && (j+k*dj < h->size)){
      int a = board__get_color(h->board, i+k*di, j+k*dj);
      if (color == a || a == -1){ row += right; }
      else{ right = 0; }
    }
    if ((0 <= i-k*di)
    && (i-k*di < h->size)
    && (0 <= j-k*dj)
    && (j-k*dj < h->size)){
      int b = board__get_color(h->board, i-k*di, j-k*dj);
      if (color == b || b == -1){ row += left; }
      else{ left = 0; }
    }
  }
  return row;
}

int explore_line(struct heuristic * h, int i, int j, int di, int dj, int color){
  int iter = 0;
  int row = 0;
  i += di;
  j += dj;
  while (0 <= i && h->size > i && 0 <= j && h->size > j && iter < 4){
    int a = board__get_color(h->board, i, j);
    if (a == color){ row += 1; }
    //if (a == (color+1)%2){ break; }
    else { break; }
    i += di;
    j += dj;
    iter++;
  }
  return row;
}

/**
 * @brief Evaluation of a point at position i,j on the board of the player color by taking into account. Return a value for the position by taking into account all directions around the position i,j (horizontal, vertical, left diagonal and right diagonal)
 * @param[in] h heuristic
 * @param[in] i row
 * @param[in] j column
 * @param[in] color id of the player
 * @return a value for a position i,j on the board
 */
int evaluate_point(struct heuristic * h, int i, int j, int color){
  int res = 0;
  int di[] = {0,1,1,1,0,-1,-1,-1};
  int dj[] = {-1,-1,0,1,1,1,0,-1};
  int power[] = {1,1,1,1};
  for(int k = 0; k < 8; k++){
      power[k%4] += explore_line(h, i, j, di[k], dj[k], color);
  }
  for(int k = 0; k < 4; k++){
    // if we cannot make a line a size at least 5, this line is useless
    if (size_line(h, i, j,di[k], dj[k], color) < 5){
      power[k] = 0;
    }
    if (power[k] > res){
      res = power[k];
    }
  }
  return res;
}

/**
 * @brief change both evaluate_point to evaluate_point_board for testing with explore_line from the bitboard 
 * @param[in/out] h heuristic
 * @param[in] i row
 * @param[in] j column
 * @return heuristic updated with matrix opportunities and dangers updated
 */
void update_potential_lines_point(struct heuristic *h, int i, int j){
   if (matrix_get(h->opportunity, i, j) != -1){
     matrix_set(h->opportunity, i, j, evaluate_point(h, i, j, h->color));
  }
  if (matrix_get(h->danger, i, j) != -1){
    matrix_set(h->danger, i, j, evaluate_point(h, i, j, (h->color+1)%2));
  }
}

/**
 * @brief Update all positions around the position i,j given (the star around the position)
 * @param[in/out] h heuristic
 * @param[in] i row
 * @param[in] j column
 * @return heuristic updated with all matrix opportunities and dangers updated around a position i,j
 */
void update_potential_lines(struct heuristic *h, int i, int j){
  for(int k = 0; k < 5; k++){
    if (j-k >= 0) {update_potential_lines_point(h, i, j-k);}
    if (j+k < h->size){update_potential_lines_point(h, i, j+k);}
    if (i-k >= 0){update_potential_lines_point(h, i-k, j);}
    if (i+k < h->size){update_potential_lines_point(h, i+k, j);}
    if (j-k >= 0 && i-k >= 0){update_potential_lines_point(h, i-k, j-k);}
    if (j+k < h->size && i+k < h->size){update_potential_lines_point(h, i+k, j+k);}
    if (i-k >= 0 && j+k < h->size){update_potential_lines_point(h, i-k, j+k);}
    if (i+k < h->size && j-k >= 0){update_potential_lines_point(h, i+k, j-k);}
  }
}

/* External function */
void heuristic_actualize(struct heuristic * h, struct move_t m, int color){
    board__add_move(h->board, m, color);
    matrix_set(h->opportunity, m.row, m.col, -1);
    matrix_set(h->danger, m.row, m.col, -1);
    update_potential_lines(h, m.row, m.col);
}

void heuristic_remove(struct heuristic * h, struct move_t m){
  board__remove_move(h->board, m);
  matrix_set(h->opportunity, m.row, m.col, 0);
  matrix_set(h->danger, m.row, m.col, 0);
  update_potential_lines(h, m.row, m.col);
}

int heuristic_full(struct heuristic * h){
    return board__is_full(h->board);
}

struct heuristic * heuristic_init(size_t size, int color){
    struct heuristic * h = malloc(sizeof(*h));
    h->board = board__initialize(size);
    h->opportunity = matrix_generate(size, 0);
    h->danger = matrix_generate(size, 0);
    h->size = size;
    h->color = color;
    return h;
}

void heuristic_free(struct heuristic * h){
    matrix_free(h->opportunity);
    matrix_free(h->danger);
    board__free(h->board);
    free(h);
}

int heuristic_eval_game(struct heuristic * h, int color){
    return matrix_sum(h->opportunity) - matrix_sum(h->danger);
}

struct list * generating_moves(struct heuristic * h, int color){
    struct matrix * opp = NULL;
    struct matrix * dan = NULL;
    if (color == h->color){
        opp = h->opportunity;
        dan = h->danger;
    }
    else{
        dan = h->opportunity;
        opp = h->danger;
    }
    struct list * l1 = matrix_list_moves_max(opp);
    struct list * l2 = matrix_list_moves_max(dan);
    if (list_length(l1) == 0 || list_length(l2) == 0){printf("error !!\n");}
    struct move_t mv = list_get(l1, 0);
    int my_force = matrix_get(opp, mv.row, mv.col);
    mv = list_get(l2, 0);
    int the_other_one_force = matrix_get(dan, mv.row, mv.col);
    if (my_force > the_other_one_force || my_force == 5 || the_other_one_force < 2){
      list_free(l2);
      return l1;
    }
    else{
      list_free(l1);
      return l2;
    }
}

int heuristic_potential_point(struct heuristic * h, struct move_t m){
    int a = matrix_get(h->opportunity, m.row, m.col);
    int b = matrix_get(h->danger, m.row, m.col);
    return a > b ? a : b;
}
