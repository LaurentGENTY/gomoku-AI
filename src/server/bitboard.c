/**
 * @file bitboard.c
 * Bitboard implementation of board.h.
 */
#include <stdlib.h>
#include "bitboard.h"

/* Direction vectors for dir = 0 (horizontal), 1 (vertical), 2 (diagonal), 3 (anti-diagonal) */
static const int DR[4] = {0, 1, 1, 1};
static const int DC[4] = {1, 0, 1, -1};

static __uint128_t cell_bit(const struct board* b, size_t row, size_t col){
  return (__uint128_t)1 << (row * b->size + col);
}

static int in_board(const struct board* b, long row, long col){
  return row >= 0 && col >= 0 && row < (long)b->size && col < (long)b->size;
}

__uint128_t board__select_bit(__uint128_t* b, int pos){
  return (*b >> (pos - 1)) & 1;
}

struct board* board__initialize(size_t size){
  if (size == 0 || size > BOARD_MAX_SIZE){
    return NULL;
  }
  struct board* b = malloc(sizeof(*b));
  b->b_w = malloc(sizeof(__uint128_t));
  b->b_b = malloc(sizeof(__uint128_t));
  b->size = size;
  b->capacity = size * size;
  /* Bits past the board are set in both integers so that "full" is a single
     comparison against all-ones. */
  __uint128_t outside = ~(__uint128_t)0 << b->capacity;
  *b->b_w = outside;
  *b->b_b = outside;
  return b;
}

void board__free(struct board* b){
  if (b == NULL){
    return;
  }
  free(b->b_w);
  free(b->b_b);
  free(b);
}

struct board* board__copy(const struct board* b){
  struct board* c = board__initialize(b->size);
  *c->b_w = *b->b_w;
  *c->b_b = *b->b_b;
  return c;
}

int board__get_color(const struct board* b, int row, int col){
  if (!in_board(b, row, col)){
    return -1;
  }
  size_t pos = (size_t)row * b->size + (size_t)col;
  if ((*b->b_b >> pos) & 1){
    return BLACK;
  }
  if ((*b->b_w >> pos) & 1){
    return WHITE;
  }
  return -1;
}

int board__is_valid_move(const struct board* b, struct move_t m){
  if (m.row >= b->size || m.col >= b->size){
    return 0;
  }
  return board__get_color(b, (int)m.row, (int)m.col) == -1;
}

void board__add_move(struct board* b, struct move_t m, enum color_t c){
  __uint128_t* stones = (c == BLACK) ? b->b_b : b->b_w;
  *stones |= cell_bit(b, m.row, m.col);
}

void board__remove_move(struct board* b, struct move_t m){
  __uint128_t keep = ~cell_bit(b, m.row, m.col);
  *b->b_b &= keep;
  *b->b_w &= keep;
}

int board__is_full(const struct board* b){
  return (*b->b_w | *b->b_b) == ~(__uint128_t)0;
}

/* Number of consecutive stones of `color` after (row, col) in direction (dr, dc) */
static int run_length(const struct board* b, int row, int col, int dr, int dc, int color){
  int n = 0;
  row += dr;
  col += dc;
  while (in_board(b, row, col) && board__get_color(b, row, col) == color){
    n++;
    row += dr;
    col += dc;
  }
  return n;
}

int board__won(const struct board* b, struct move_t m){
  if (m.row >= b->size || m.col >= b->size){
    return 0;
  }
  int row = (int)m.row;
  int col = (int)m.col;
  int color = board__get_color(b, row, col);
  if (color == -1){
    return 0;
  }
  for (int d = 0; d < 4; d++){
    int n = 1 + run_length(b, row, col, DR[d], DC[d], color)
              + run_length(b, row, col, -DR[d], -DC[d], color);
    if (n >= 5){
      return 1;
    }
  }
  return 0;
}

void board__explore_line(const struct board* b, int row, int col, int dir, int color, int* pattern){
  (void)b; (void)row; (void)col; (void)dir; (void)color; (void)pattern;
}
