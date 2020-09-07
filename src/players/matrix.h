/**
 * @file matrix.h
 */

#ifndef MATRIX_H
#define MATRIX_H
#include "stdlib.h"
#include "list.h"

/**
 * @brief Implementation of the strenghs of all positions for a given board. At a moment T during the game, heuristic will create matrix for the player and oppponent in order to retrieve all strenghs for plays in this matrix
 */
struct matrix{
  /**
   * matrix representation of size size*size. Each int in m represents the value of the strengh of the play
   */
  int * m;
  /**
   * size of the board
   */
  size_t size;
};

/**
 * @brief Generate a matrix of size size filled the default value given in parameter
 * @param[in] size the size of the board
 * @param[in] value default value for each position in the matrix
 * @return matrix allocated in the heap of size size with a default value
 */
struct matrix * matrix_generate(size_t size, int value);

/**
 * @brief Return the value at position i,j of the matrix
 * @param[in] i row
 * @param[in] j column
 * @return value at position i,j in the matrix if it exists, 0 otherwise
 */
int matrix_get(struct matrix * m, int i, int j);

/**
 * @brief Set a value at a position i,j in the matrix
 * @param [in/out] m matrix
 * @param [in] i row
 * @param [in] j column
 * @param [in] value value at new position
 * @return matrix with the new value at position i,j
 */
void matrix_set(struct matrix * m, int i, int j, int value);

/**
 * @brief Add the value given in parameter to the old value in matrix at position i,j. This fonction is different from matrix_set because here, the value given in parameter is added to the old one
 * @param [in/out] m matrix
 * @param [in] i row
 * @param [in] j column
 * @param [in] value value added
 * @return matrix with the value added at position i,j
 */
void matrix_add(struct matrix * m, int i, int j, int value);

/**
 * @brief Find the maximum value in a matrix
 * @param [in] m matrix
 * @return maximal value in a matrix (can be -1 or a positive int)
 */
int matrix_max(struct matrix * m);

/**
 * @brief Create in the heap and return a list of moves and add all maximum values of the matrix at position i,j in it. All values in the list are the same, only position are different
 * @param [in] m matrix
 * @return a list full of positions with maximal values of a matrix
 */
struct list * matrix_list_moves_max(struct matrix * m);

/**
 * @brief Create in the heap and return a list of moves and add all minimum values of the matrix at position i,j in it. All values in the list are the same, only position are different
 * @param [in] m matrix
 * @return list full of positions with minimal values of a matrix
 */
void matrix_list_moves_min(struct matrix * m, struct list * moves);

/**
 * @brief Fill all the matrix with a default value
 * @param [in/out] m matrix
 * @param [in] value default value
 * @return matrix filled with the default value
 */
void matrix_clear(struct matrix * m, int value);

/**
 * @brief Copy a matrix in the heap
 * @param [in] m matrix
 * @return a copy of the matrix allocated in the heap
 */
struct matrix * matrix_copy(struct matrix * m);

/**
 * @brief Return the sum of all values in a matrix
 * @param [in] m matrix
 * @return the sum of all values in the matrix
 */
int matrix_sum(struct matrix * m);

/**
 * @brief Computes all occurrences of all values of a matrix in a table s
 * @param[in] m matrix
 * @param[in/out] s table where all occurrences are stored
 * @return s full of values for occurrences
 */
void matrix_number(struct matrix * m, int * s);

/**
 * @brief Return a list of good moves in a matrix. After generating the number of occurrences with matrix_number this functions provides a list where are picked nb_moves best moves in it.
 * @param[in] m matrix
 * @param[in] max_value 
 * @param[in] layers
 * @param[in] nb_moves number of moves wanted in the list
 * @return list with nb_moves good moves in the matrix 
 */
struct list * matrix_good_moves(struct matrix * m, int max_value, int layers, int nb_moves);

/**
 * @brief Free a matrix allocated in the heap
 * @param [in/out] m matrix
 * @return matrix is no longer allocated in the heap
 */
void matrix_free(struct matrix * m);

/**
 * @brief Print a matrix on the terminal in a lovely way
 * @param [in] m matrix
 */
void matrix_print(struct matrix *m);

#endif
