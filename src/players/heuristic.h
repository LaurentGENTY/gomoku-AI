/**
 * @file heuristic.h
 *
 */

#ifndef HEUR_H
#define HEUR_H

#include "matrix.h"
#include "../server/board.h"

#include "stdlib.h"

/**
 * @brief Player that will play on a board by giving successive moves to the server
 */
struct heuristic{
  /**
   * own board for the heuristic
   */
  struct board * board;
  /**
   * matrix containing all possible rows that are opportunities for the player (filled with positive or -1 values)
   */
  struct matrix * opportunity;
  /**
   * matrix containing all enemy possible rows that are dangers for the player (filled with positive or -1 values)
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
};

/**
 * @brief Initialize the heuristic in the heap with the size of the board, the id of the player, both matrixes for opportunities and dangers initialized
 * @param[in] size size of the board
 * @param[in] color id of the player
 * @return heuristic initialized in the heap with initialized matrixes inside
 */
struct heuristic * heuristic_init(size_t size, int color);

/**
 * @brief Free the heuristic
 * @param[in/out] h heuristic that needs to be freed
 * @return heuristic freed
 */
void heuristic_free(struct heuristic * h);

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

#endif
