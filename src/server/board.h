/**
 * @file board.h
 * Bitboard: one 128-bit integer per color, one bit per cell (row-major), so
 * boards are limited to 11x11 = 121 cells. Rebuilt from the project report
 * and from the way the players use it (the original sources were lost).
 */
#ifndef BOARD_H
#define BOARD_H

#include <stddef.h>
#include "move.h"

#define BOARD_MAX_SIZE 11

struct board {
  __uint128_t* b_w;
  __uint128_t* b_b;
  size_t capacity;
  size_t size;
};

struct board* board__initialize(size_t size);
void board__free(struct board* b);
struct board* board__copy(const struct board* b);

int board__is_valid_move(const struct board* b, struct move_t m);
void board__add_move(struct board* b, struct move_t m, enum color_t c);
void board__remove_move(struct board* b, struct move_t m);
int board__is_full(const struct board* b);

/** @return -1 for an empty or off-board cell, BLACK or WHITE otherwise */
int board__get_color(const struct board* b, int row, int col);

/** @return 1 if the stone at m is part of 5 or more aligned stones */
int board__won(const struct board* b, struct move_t m);

/**
 * Pattern that `color` would form by playing the empty cell (row, col), along
 * one direction (0 horizontal, 1 vertical, 2 diagonal, 3 anti-diagonal).
 * Increments at most one entry of pattern[9], indexed as:
 * FIVE, FOUR_OPEN, FOUR_HALF, FOUR_SPACED, THREE_OPEN, THREE_HALF,
 * TWO_OPEN, TWO_HALF, OTHERS. Nothing is incremented when the line can never
 * reach five stones.
 */
void board__explore_line(const struct board* b, int row, int col, int dir, int color, int* pattern);

#endif
