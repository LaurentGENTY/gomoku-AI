/**
 * @file heuristic2.h
 * Heuristic using minmax strategy which an improvement of the heuristic.h. Here compared to the previous heuristic, when the player has several plays with a same strengh then deepen in the tree of possibilities
 */
#ifndef HEUR_H
#define HEUR_H

#include <stdlib.h>

#include "matrix.h"
#include "../server/board.h"

/* Maximal value for evaluation of a point */
#define MAX_VAL 13

/* Variable that caracterize of 100% sure winning play */
#define INFINITY 2147483647

/**
 * @brief Heuristic with 2 matrix based on the board for player and the opponent which will computes all moves for the player and try to find the best one based of different parameters (depth, nb of childs visited, layers...). It does the same as previous strategies as this heuristic tries to play the best move for the player or counterplays the opponent if his best moves has better strenghs.
 */
struct heuristic{
  /**
   * own board for the heuristic
   */
  struct board * board;
  /**
   * matrix containing all possible rows that are opportunities for the player
   */
  struct matrix * opportunity;
  /**
   * matrix containing all enemy possible rows that are dangers for the player
   */
  struct matrix * danger;
  /**
   * size of the board
   */
  size_t size;
  /**
   * color of the player (id)
   */
  int color;
  /**
   * the depth of the travelled tree
   */
  int depth;
  /**
   * number of childs the minmax is checking
   */
  int nb_childs;
  /**
   * number of layers in order to choose number of nodes for a list of good moves
   */
  int nb_layers;
  /**
   * number of nodes explored during negamax for statistics
   */
  int nb_node_explored;
};

/**
 * @brief Initialize a heuristic in the heap with the size of the board, the id of the player, depth of the minmax tree, maximal number of childs seen and the number of layers
 * @param[in] size size of the board
 * @param[in] color id of the player
 * @param[in] depth depth of the minmax tree
 * @param[in] nb_childs maximal number of childs checked for the minmax strategy
 * @param[in] nb_layers number of layers
 * @return heuristic initialized in the heap
 */
struct heuristic * heuristic_init(size_t size, int color, int depth, int nb_childs, int nb_layers);

/**
 * @brief Free the heuristic
 * @param[in/out] h heuristic that needs to be freed
 * @return heuristic freed
 */
void heuristic_free(struct heuristic * h);

/**
 * @brief Copy an original heuristic in the heap
 * @param[in] h heuristic
 * @return copy of the previous heuristic allocated in the heap
 */
struct heuristic * heuristic_copy(struct heuristic * h);

/**
 * @brief Return the value of the last move added with heuristic_actualize. We know the length of our last line made
 * @param[in] h heuristic where the board will be evaluated
 * @param[in] color id of the player
 * @return int value of the current board which help to choose the best board that will be played
 */
int heuristic_eval_game(struct heuristic * h, int color);

/**
 * @brief Extract the best moves from the matrix of opportunities and matrix of dangers (with the maximum evaluation) and return the list of best moves for the player that are better for us (opportunities) than for the opponent (dangers)
 * @param[in] h heuristic where matrix opportunities and dangers are checked
 * @param[in] color id of the player
 * @return return the list of best moves for the player that are better for us (opportunities) than for the opponent (dangers)
 */
struct list * generating_moves(struct heuristic * h, int color);

/**
 * @brief Update the heuristic's board and 2 matrixes with the move given in parameter
 * @param[in/out] h heuristic that will be updated (board and matrixes inside)
 * @param[in] m added move
 * @param[in] color id pf the player
 * @return self board of the heuristic internally actualized (board and matrixes) with the new move added
 */
void heuristic_actualize(struct heuristic * h, struct move_t m, int color);

/**
 * @brief Undo what was done by heuristic_actualize by removing a move from the heuristic board
 * @param[in] h heuristic where we will remove the move from board and matrixe
 * @param[in] mv move we want to remove from the heuristic's board
 * @return heuristic's board updated without the move just removed
 */
void heuristic_remove(struct heuristic * h, struct move_t mv);

/**
 * @brief Verify if the board is full
 * @param[in] h heuristic where the board will be retrieved
 * @return 1 if the heuristic's board is full, 0 otherwise
 */
int heuristic_full(struct heuristic * h);

/**
 * @brief Return the evaluation that was made for the given move
 * @param[in] h heuristic where the evaluation of points will be computed
 * @param[in] m move computed
 * @return evaluation of the point on the heuristic's board
 */
int heuristic_potential_point(struct heuristic * h, struct move_t m);

/**
 * @brief Start a minmax strategy on the current board with a depth given in parameter. It is a recursive function which explore the tree with a depth given in parameter (and subbed to 1 for each recursive call). When the depth is null, the board will be evaluated. Then, after returning the evalution of the board, it will choose the maximum (minimum) value for the player (and the opponent). That is the strategy of minmax
 * @param[in] h heuristic
 * @param[in] depth depth of the minmax exploring
 * @param[in] alpha value in order to memorize previous values of previous opportunities
 * @param[in] beta value
 * @param[in] color id of the player in order to know which maximum or minimum value is taken
 * @return the best move with a depth given on a current board for a player
 */
int negamax(struct heuristic * h, int depth, int alpha, int beta, int color);

#endif
