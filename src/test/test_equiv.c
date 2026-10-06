/**
 * @file test_equiv.c
 * Prints the moves player 4.4 picks on fixed positions. Built twice (with and
 * without GOMOKU_NO_THREADS); `make test-equiv` checks both builds agree.
 */
#include <stdio.h>
#include "../players/player.h"

#define SIZE 10

struct position {
  enum color_t to_play;
  size_t n;
  struct col_move_t moves[8];
};

static const struct position positions[] = {
  { WHITE, 1, {{{4,4},BLACK}} },
  { BLACK, 2, {{{4,4},BLACK},{{4,5},WHITE}} },
  { BLACK, 4, {{{4,4},BLACK},{{5,5},WHITE},{{4,5},BLACK},{{3,3},WHITE}} },
  { WHITE, 5, {{{4,4},BLACK},{{5,5},WHITE},{{4,5},BLACK},{{3,3},WHITE},{{4,6},BLACK}} },
  { BLACK, 6, {{{2,2},BLACK},{{7,7},WHITE},{{2,3},BLACK},{{7,6},WHITE},{{3,3},BLACK},{{6,6},WHITE}} },
};

int main(void){
  size_t count = sizeof(positions) / sizeof(positions[0]);
  for (size_t i = 0; i < count; i++){
    initialize(SIZE, positions[i].to_play);
    struct move_t m = play(positions[i].moves, positions[i].n);
    printf("MOVE %zu %zu %zu\n", i, m.row, m.col);
    finalize();
  }
  return 0;
}
