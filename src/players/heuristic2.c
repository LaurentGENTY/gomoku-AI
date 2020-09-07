/**
 * @file heuristic2.c
 * Implementation of heuristic2.h for minmax player (player4.3.so)
 */

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"
#include "list.h"
#include "heuristic2.h"
#include "../server/board.h"

/* Internal functions */

/**
 * @brief Evaluation of a move at position i,j on the board of the player color by taking into account. Return a value for the position by taking into account all directions around the position i,j (horizontal, vertical, left diagonal and right diagonal)
 * @param[in] h heuristic
 * @param[in] i row
 * @param[in] j column
 * @param[in] color id of the player
 * @return a value for a position i,j on the board
 */
int evaluate_move_board(struct heuristic * h, int i, int j, int color){
  int pattern[9] = {0,0,0,0,0,0,0,0,0};
  enum pat { FIVE=0, FOUR_OPEN=1, FOUR_HALF=2, FOUR_SPACED = 3, THREE_OPEN=4, THREE_HALF=5, TWO_OPEN=6, TWO_HALF=7, OTHERS= 8};
  for(int k = 0; k < 4; k++){
    board__explore_line(h->board, i, j, k, color,pattern);
  }
  int typeMark[]={13,12,11,10,9,8,7,6,5,4,3,2,1,0};
  if(pattern[FIVE]!=0){
    return typeMark[0];
  }else if((pattern[FOUR_OPEN]!=0)||				\
	   ((pattern[FOUR_HALF]+pattern[FOUR_SPACED])>=2)||		\
	   (((pattern[FOUR_HALF]+pattern[FOUR_SPACED])!=0)&&(pattern[THREE_OPEN]!=0))){
    return typeMark[1];
  }else if(pattern[THREE_OPEN]>=2){
    return typeMark[2];
  }else if ((pattern[THREE_OPEN]!=0)&&(pattern[THREE_HALF]!=0)){
    return typeMark[3];
  }else if (pattern[THREE_OPEN]!=0){
    return typeMark[4];
  }else if (pattern[FOUR_HALF]!=0){
    return typeMark[5];
  }else if (pattern[FOUR_SPACED]!=0){
    return typeMark[6];
  }else if(pattern[TWO_OPEN]>=2){
    return typeMark[7];
  }else if(pattern[THREE_HALF]!=0){
    return typeMark[8];
  }else if((pattern[TWO_OPEN]!=0)&&(pattern[TWO_HALF]!=0)){
    return typeMark[9];
  }else if(pattern[TWO_OPEN]!=0){
    return typeMark[10];
  }else if(pattern[TWO_HALF]!=0){
    return typeMark[11];
  }else if(pattern[OTHERS]!=0){
    return typeMark[12];
  }else{
    return 0;
  }
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
     int a = evaluate_move_board(h, i, j, h->color);
     matrix_set(h->opportunity, i, j, a);
  }
  if (matrix_get(h->danger, i, j) != -1){
    int a = evaluate_move_board(h, i, j, (h->color+1)%2);
    matrix_set(h->danger, i, j, a);
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

/* External functions */

struct heuristic * heuristic_init(size_t size, int color, int depth, int nb_childs, int nb_layers){
    struct heuristic * h = malloc(sizeof(*h));
    h->board = board__initialize(size);
    h->opportunity = matrix_generate(size, 0);
    h->danger = matrix_generate(size, 0);
    h->size = size;
    h->color = color;
    h->depth = depth;
    h->nb_childs = nb_childs;
    h->nb_layers = nb_layers;
    h->nb_node_explored = 0;
    return h;
}

void heuristic_free(struct heuristic * h){
    matrix_free(h->opportunity);
    matrix_free(h->danger);
    board__free(h->board);
    free(h);
}

struct heuristic * heuristic_copy(struct heuristic * h){
    struct heuristic * new = malloc(sizeof(*new));
    new->board = board__copy(h->board);
    new->opportunity = matrix_copy(h->opportunity);
    new->danger = matrix_copy(h->danger);
    new->size = h->size;
    new->color = h->color;
    new->depth = h->depth;
    new->nb_childs = h->nb_childs;
    new->nb_layers = h->nb_layers;
    new->nb_node_explored = h->nb_node_explored;
    return new;
}

int heuristic_eval_game(struct heuristic * h, int color){
    if (color == h->color){
        return matrix_sum(h->opportunity) - matrix_sum(h->danger);
    }
    return matrix_sum(h->danger) - matrix_sum(h->opportunity);
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
    struct list * l1 = matrix_good_moves(opp, MAX_VAL, h->nb_layers, h->nb_childs);
    struct list * l2 = matrix_good_moves(dan, MAX_VAL, h->nb_layers, h->nb_childs);
    struct move_t mv = list_get(l1, 0);    // the first move is the best
    int my_force = matrix_get(opp, mv.row, mv.col);
    mv = list_get(l2, 0);
    int the_other_one_force = matrix_get(dan, mv.row, mv.col);
    if (my_force == MAX_VAL || the_other_one_force < 2){
      list_free(l2);
      return l1;
    }
    list_concat(l1, l2); //mais ya des doublons !!
    list_free(l2);
    return l1;
}

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

int heuristic_potential_point(struct heuristic * h, struct move_t m){
    int a = matrix_get(h->opportunity, m.row, m.col);
    int b = matrix_get(h->danger, m.row, m.col);
    return a > b ? a : b;
}

int negamax(struct heuristic * h, int depth, int alpha, int beta, int color){
    h->nb_node_explored++;
    if (depth == 0 || heuristic_full(h)){
      return heuristic_eval_game(h, color);
    }
    struct list * moves = generating_moves(h, color);
    if (heuristic_potential_point(h, list_get(moves, 0)) >= MAX_VAL){
      list_free(moves);
      return INFINITY-1;
    }
    int step;
    for(int i = 0; i < list_length(moves); i++){
      struct move_t mv = list_get(moves, i);
      heuristic_actualize(h, mv, color); // simulate the move
      step = -negamax(h, depth-1, -beta, -alpha, (color+1)%2);
      heuristic_remove(h, mv); //undo what was done
      if (step > alpha){ alpha = step; }
      if (beta <= alpha){ break; }
    }
    list_free(moves);
    return alpha;
}
