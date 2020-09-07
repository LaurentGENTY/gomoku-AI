/**
 * @file player.h
 */

#ifndef _BITBOARD_PLAYER_H_
#define _BITBOARD_PLAYER_H_

#include <stddef.h>
#include "../server/move.h"

/* Public functions */

/**
 * @brief Access to player informations
 * @return the player name as an [a-zA-Z0-9 -_]* string
 */
char const* get_player_name();

/**
 * @brief Returns a sequcen of three moves for an opening
 * @param[in] size the size of the board, size >= 5
 * @return an heap-allocated pointer to an array of three valid moves, that are consecutively BLACK, WHITE and BLACK.
 */
struct col_move_t* propose_opening(size_t size);

/**
 * @brief Accept the opening or not depending on the moves on the board
 * @param[in] size the size of the board, size >= 5
 * @param[in] opening a pointer to an array of three moves, the size of the array is at least 3 and the moves sequence is BLACK, WHITE and BLACK
 * @return 1 if the player accepts the opening, 0 otherwise
 */
int accept_opening(size_t size, const struct col_move_t* opening);

/**
 * @brief Initialize the player: store the size of the board, the id (color)
 * @param[in] size the size of the board, size >= 5
 * @param[in] id either BLACK or WHITE
 * @return player internally initialized with the size of the board and the color
 */
void initialize(size_t size, enum color_t id);

/**
 * @brief Computes the next move by taking into account the last n moves. Different players have differents strategies which means each player won't return the same move insofar as they treat the board differently
 * @param previous_moves[] ordered list of previous moves starting from the last move issued by the player if it exists, otherwise from the beginning of the game (opening moves are not considered as moves issued by a particular player, even if they have a color). It is an arrat of at least n_moves elements
 * @param n_moves number of moves in previous_moves
 * @return the next computed move for the player
 */
struct move_t play(struct col_move_t const previous_moves[], size_t n_moves);

/**
 * @brief Announces the end of the game to the player, and cleans up the memory he may have been using
 * @return every allocation done during the calls to initialeze and play functions must have been freed
 */
void finalize();

#endif // _BITBOARD_PLAYER_H_
