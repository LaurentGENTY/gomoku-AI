/**
 * @file moves.h
 * Last moves of a game, as the server sent them to the players: the 3 opening
 * moves plus the first real move, then the last 2 moves of each turn.
 */
#ifndef MOVES_H
#define MOVES_H

#include <stddef.h>
#include "move.h"
#include "board.h"

#define MOVES_CAPACITY 4

struct moves {
  struct col_move_t t[MOVES_CAPACITY];
  size_t size;
};

struct moves* moves__initialize(void);
void moves__free(struct moves* m);
size_t moves__get_size(const struct moves* m);
/** @return the stored moves, oldest first (never NULL) */
struct col_move_t* moves__last_n(struct moves* m);
void moves__enqueue(struct moves* m, const struct col_move_t* move);
/** Stores the 3 opening moves, plays them on the board and frees `opening`. */
void moves__add_opening(struct col_move_t* opening, struct moves* m, struct board* b);

#endif
