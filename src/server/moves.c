/**
 * @file moves.c
 * Implementation of moves.h.
 */
#include <stdlib.h>
#include "moves.h"

struct moves* moves__initialize(void){
  struct moves* m = malloc(sizeof(*m));
  m->size = 0;
  return m;
}

void moves__free(struct moves* m){
  free(m);
}

size_t moves__get_size(const struct moves* m){
  return m->size;
}

struct col_move_t* moves__last_n(struct moves* m){
  return m->t;
}

void moves__enqueue(struct moves* m, const struct col_move_t* move){
  /* Once a turn is complete (2 moves, or 4 right after an opening) only the
     latest move is kept: a player receives its own last move and the reply. */
  if (m->size == 2 || m->size == MOVES_CAPACITY){
    m->t[0] = m->t[m->size - 1];
    m->size = 1;
  }
  m->t[m->size++] = *move;
}

void moves__add_opening(struct col_move_t* opening, struct moves* m, struct board* b){
  for (int i = 0; i < 3; i++){
    m->t[i] = opening[i];
    board__add_move(b, opening[i].m, opening[i].c);
  }
  m->size = 3;
  free(opening);
}
