/**
 * @file move.h
 * Moves and colors shared by the board, the players and the tests.
 */
#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>

enum color_t { BLACK = 0, WHITE = 1 };

struct move_t {
  size_t row;
  size_t col;
};

struct col_move_t {
  struct move_t m;
  enum color_t c;
};

#endif
