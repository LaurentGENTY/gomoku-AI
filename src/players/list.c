/**
 * @file list.c
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "list.h"
#include "../server/move.h"

#define MIN_SIZE 5

struct list * list_generate(){
  struct list * l = malloc(sizeof(*l));
  l->len = 0;
  l->capacity = MIN_SIZE;
  l->s = malloc(sizeof(struct move_t) * MIN_SIZE);
  return l;
}

int list_empty(struct list * l){
  assert(l);
  return l->len == 0;
}

int list_length(struct list * l){
  assert(l);
  return l->len;
}

void list_add(struct list * l, size_t i, size_t j){
  assert(l);

  /* If we don't have enough capacity for the list */
  if (l->len == l->capacity){
    l->capacity *= 2;
    l->s = realloc(l->s, sizeof(struct move_t) * l->capacity);
  }
  l->s[l->len].row = i;
  l->s[l->len].col = j;
  l->len++;
}

struct move_t list_get(struct list * l, int i){
  assert(l);
  return l->s[i];
}

int list_find(struct list * l, struct move_t mv){
  assert(l);

  struct move_t m;
  for(size_t i = 0; i < l->len; i++){
    m = list_get(l, i);
    if (m.row == mv.row && m.col == mv.col){
      return 1;
    }
  }
  return 0;
}

void list_concat(struct list * l1, struct list * l2){
  assert(l1);
  assert(l2);

  for(size_t i = 0; i < l2->len; i++){
    struct move_t mv = list_get(l2, i);
    if (!list_find(l1, mv)){
      list_add(l1, mv.row, mv.col);
    }
  }
}

void list_cut(struct list * l, int nb){
  assert(l);
  if (l->len > nb){
    l->len = nb;
  }
}

void list_free(struct list * l){
  assert(l);
  free(l->s);
  free(l);
}
