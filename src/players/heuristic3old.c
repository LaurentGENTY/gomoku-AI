/**
 * @file heuristic3.c
 * Implementation of minmax strategy for player4.4
 */

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"
#include "list.h"
#include "heuristic3.h"
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
    if (i+k < h->size && j-k < h->size){update_potential_lines_point(h, i+k, j-k);}
  }
}

/**
 * @brief Push a move in the stack if the move is not already in it
 * @param[in/out] h heuristic
 * @param[in] mv move added into the stack
 * @param[in] a beginning cursor in the stack
 * @param[in] b endind cursor in the stack
 * @return stack in the heuristic updated with the new move pushed in it
 */
void push(struct heuristic * h, struct move_t mv, int a, int b){
  for(int i = a; i < b; i++){
    if (h->stack[i].row == mv.row && h->stack[i].col == mv.col){
      return;
    }
  }
  h->stack[h->cursor] = mv;
  h->cursor++;
}

/**
 * @brief Generate moves for the player given in parameter
 * @param[in/out] h heuristic
 * @param[in] me id of the player
 * @param[in] max_value maximum value for the stack
 * @param[in] layers number of layers
 * @param[in] nb_moves number of potential moves added in the stack
 * @param[in] nb_previous
 */
void push_good_moves(struct heuristic * h, int me, int max_value, int layers, int nb_moves, int nb_previous){
  int start = h->cursor;
  struct move_t moves[MAX_VAL+1][121];
  int cursors[MAX_VAL+1] = {0};
  //struct move_t to_push[nb_moves];
  //int nb_to_push = 0;
  struct matrix * m = me ? h->opportunity : h->danger;
  for(size_t i = 0; i < m->size; i++){
    for(size_t j = 0; j < m->size; j++){
      int a = m->m[i * m->size + j];
      if (a > -1){
        struct move_t mv = {i,j};
        moves[a][cursors[a]] = mv;
        cursors[a]++;
      }
    }
  }
  int nb_moves_taken = 0;
  if (cursors[max_value] > 0){
    layers = 0; // victory point detected, no need in returning other moves
    for (int i = 0; i < cursors[max_value]; i++){
      //to_push[nb_to_push] = moves[max_value][i];
      //nb_to_push++;
      push(h, moves[max_value][i], start-nb_previous, start);
    }
  }
  else{
    for(int i = max_value-1; i >= 0; i--){
      if (layers > 0 && nb_moves_taken < nb_moves && cursors[i] > 0){
        for (int j = 0; j < cursors[i]; j++){
          if (nb_moves_taken < nb_moves){
            //to_push[nb_to_push] = moves[i][j];
            //nb_to_push++;
            push(h, moves[i][j], start-nb_previous, start);
            nb_moves_taken++;
          }
        }
        layers--;
      }
    }
  }
  //for(int i = nb_to_push-1; i >= 0; i--){
//    push(h, to_push[i], start-nb_previous, start);
  //}
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
  h->cursor = 0;
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
  new->cursor = h->cursor;
  new->nb_node_explored = h->nb_node_explored;
  for(int i = 0; i < h->cursor; i++){
    new->stack[i] = h->stack[i];
  }
  return new;
}

int heuristic_eval_game(struct heuristic * h, int color){
  if (color == h->color){
    return matrix_sum(h->opportunity) - matrix_sum(h->danger);
  }
  return matrix_sum(h->danger) - matrix_sum(h->opportunity);
}

void generating_moves(struct heuristic * h, int color){
  struct matrix * opp = NULL;
  int me = 1;
  if (color == h->color){
    opp = h->opportunity;
  }
  else{
    opp = h->danger;
    me = 0;
  }
  int first_opp = h->cursor; //the first move will be the best
  push_good_moves(h, me, MAX_VAL, h->nb_layers, h->nb_childs,0);
  int my_force = matrix_get(opp, h->stack[first_opp].row, h->stack[first_opp].col);
  int first_dan = h->cursor;
  push_good_moves(h, !me, MAX_VAL, h->nb_layers, h->nb_childs, h->cursor-first_opp);
  if (my_force == MAX_VAL){
    h->cursor = first_dan; // erase the moves for the ennemi, we have a clear advantage
  }
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
  int first_cursor = h->cursor;
  generating_moves(h, color); // push moves
  int step;
  while(h->cursor > first_cursor){
    struct move_t mv = pop(h);        //decrease cursor
    if (heuristic_potential_point(h, h->stack[first_cursor]) >= MAX_VAL){
      step = INFINITY-1;
    }
    else{
      heuristic_actualize(h, mv, color); // simulate the move
      step = -negamax(h, depth-1, -beta, -alpha, (color+1)%2);
      heuristic_remove(h, mv); //undo what was done
    }
    if (step > alpha){ alpha = step; }
    if (beta <= alpha){ break; }
  }
  h->cursor = first_cursor; // rappatrier le cursor si on est sorti à cause du break
  return alpha;
}

struct move_t pop(struct heuristic * h){
  h->cursor--;
  return h->stack[h->cursor];
}
