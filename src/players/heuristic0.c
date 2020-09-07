/**
* @file heuristic0.c
* Implementation of heuristic0.h for the player3 (no minmax, it is only an evaluation for each turn)
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "heuristic0.h"
#include "matrix.h"

/* Self player in order to get the board and all informations to choose what is the best for the player or not */
extern struct player self;

int heuristic_eval_game(struct matrix * opp, struct matrix * dan){
  assert(opp);
  assert(dan);
  return matrix_sum(opp) - matrix_sum(dan);
}

int size_line(int i, int j, int di, int dj, int color){
  int row = 1;
  int right = 1;
  int left = 1;
  int adi = di;
  int adj = dj;
  for(int k = 0; k < 4; k++){
    if (0 <= i+di && i+di < self.size && 0 <= j+dj && j+dj < self.size){
      int a = matrix_get(self.actual_board, i+di, j+dj);
      if (color == a || a == -1){ row += right; }
      else{ right = 0; }
    }
    if (0 <= i-di && i-di < self.size && 0 <= j-dj && j-dj < self.size){
      int b = matrix_get(self.actual_board, i-di, j-dj);
      if (color == b || b == -1){ row += left; }
      else{ left = 0; }
    }
    di += adi;
    dj += adj;
  }
  return row;
}

int explore_line(int i, int j, int di, int dj, int color){
  int iter = 0;
  int row = 0;
  i += di;
  j += dj;
  while (0 <= i && self.size > i && 0 <= j && self.size > j && iter < 4){
    int a = matrix_get(self.actual_board, i, j);
    if (a == color){ row += 1; }
    //if (a == (color+1)%2){ break; }
    else { break; }
    i += di;
    j += dj;
    iter++;
  }
  return row;
}

int evaluate_point(int i, int j, int color){
  int res = 0;
  int di[] = {0,1,1,1,0,-1,-1,-1};
  int dj[] = {-1,-1,0,1,1,1,0,-1};
  int power[] = {1,1,1,1};
  for(int k = 0; k < 8; k++){
    power[k%4] += explore_line(i, j, di[k], dj[k], color);
  }
  for(int k = 0; k < 4; k++){
    /* if we cannot make a line a size at least 5, this line is useless */
    if (size_line(i,j,di[k], dj[k], color) < 5){
      power[k] = 0;
    }
    if (power[k] > res){
      res = power[k];
    }
  }
  return res;
}

void update_potential_lines(int i, int j, struct matrix * m, int color){
  assert(m);
  for(int k = 0; k < 5; k++){
    if (j-k >= 0) {update_potential_lines_point(m, i, j-k, color);}
    if (j+k < self.size){update_potential_lines_point(m, i, j+k, color);}
    if (i-k >= 0){update_potential_lines_point(m, i-k, j, color);}
    if (i+k < self.size){update_potential_lines_point(m, i+k, j, color);}
    if (j-k >= 0 && i-k >= 0){update_potential_lines_point(m, i-k, j-k, color);}
    if (j+k < self.size && i+k < self.size){update_potential_lines_point(m, i+k, j+k, color);}
    if (i-k >= 0 && j+k < self.size){update_potential_lines_point(m, i-k, j+k, color);}
    if (i+k < self.size && j-k < self.size){update_potential_lines_point(m, i+k, j-k, color);}
  }
}

void update_potential_lines_point(struct matrix * m, int i, int j, int color){
  assert(m);
  if (matrix_get(m, i, j) != -1){
    matrix_set(m, i, j, evaluate_point(i,j,color));
  }
}

struct list * generating_moves(struct matrix * opp, struct matrix * dan){
  assert(opp);
  struct list * l1 = matrix_list_moves_max(opp);
  struct list * l2 = matrix_list_moves_max(dan);
  if (list_length(l1) == 0 || list_length(l2) == 0){printf("error !!\n");}
  struct move_t mv = list_get(l1, 0);
  int my_force = matrix_get(opp, mv.row, mv.col);
  mv = list_get(l2, 0);
  int the_other_one_force = matrix_get(dan, mv.row, mv.col);
  if (my_force > the_other_one_force || my_force == 5){
    list_free(l2);
    return l1;
  }
  else{
    list_free(l1);
    return l2;
  }
}

void heuristic_actualize(struct move_t m, int color){
  matrix_set(self.actual_board, m.row, m.col, color);
  matrix_set(self.opportunity, m.row, m.col, -1);
  matrix_set(self.danger, m.row, m.col, -1);
  update_potential_lines(m.row, m.col, self.opportunity, self.color);
  update_potential_lines(m.row, m.col, self.danger, (self.color+1)%2);
  self.nb_moves++;
}

void heuristic_remove(struct move_t m){
  matrix_set(self.actual_board, m.row, m.col, -1);
  matrix_set(self.opportunity, m.row, m.col, 0);
  matrix_set(self.danger, m.row, m.col, 0);
  update_potential_lines(m.row, m.col, self.opportunity, self.color);
  update_potential_lines(m.row, m.col, self.danger, (self.color+1)%2);
  self.nb_moves--;
}

int heuristic_full(){
  return self.nb_moves == self.size * self.size;
}
