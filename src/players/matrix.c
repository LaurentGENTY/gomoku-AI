/**
 * @file matrix.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "matrix.h"
#include "list.h"

struct matrix * matrix_generate(size_t size, int value){
  struct matrix * m = malloc(sizeof(*m));
  m->size = size;
  m->m = malloc(size * size * sizeof(int*));
  for(size_t i = 0; i < size*size; i++){
    m->m[i] = value;
  }
  return m;
}

int matrix_get(struct matrix * m, int i, int j){
  if (i + m->size * j < m->size * m->size){
    return m->m[i * m->size + j];
  }
  return -2; // return -2 by default
}

void matrix_set(struct matrix * m, int i, int j, int value){
  m->m[i * m->size + j] = value;
}

void matrix_add(struct matrix * m, int i, int j, int value){
  m->m[i * m->size + j] += value;
}

int matrix_max(struct matrix * m){
  int max = m->m[0];
  for(size_t i=0; i < m->size * m->size; i++){
    if (m->m[i] > max){
      max = m->m[i];
    }
  }
  return max;
}

struct list * matrix_list_moves_max(struct matrix * m){
  struct list * moves = list_generate();
  int max = matrix_max(m);
  for(int i = 0; i < m->size; i++){
    for(int j = 0; j < m->size; j++){
      if (m->m[i * m->size + j] == max){
        list_add(moves, i, j);
      }
    }
  }
  return moves;
}

void matrix_list_moves_min(struct matrix * m, struct list * moves){
  for(int i = 0; i < m->size; i++){
    for(int j = 0; j < m->size; j++){
      if (m->m[i * m->size + j] == -1){ //-1 is the min value
        list_add(moves, i, j);
      }
    }
  }
}


struct list * matrix_good_moves(struct matrix * m, int max_value, int layers, int nb_moves){
  struct list ** moves = malloc(sizeof(*moves) * (max_value+1));
  for(int i = 0; i < max_value+1; i++){
    moves[i] = list_generate();
  }
  for(size_t i = 0; i < m->size; i++){
    for(size_t j = 0; j < m->size; j++){
      int a = m->m[i * m->size + j];
      if (a > -1){
	list_add(moves[a], i , j);
      }
    }
  }
  struct list * res = list_generate();
  if (list_length(moves[max_value]) > 0){
    list_concat(res, moves[max_value]);
    layers = 0; // victory point detected, no need in returning other moves
  }
  for(int i = max_value; i >= 0; i--){
    if (layers > 0 && list_length(res) < nb_moves && list_length(moves[i]) > 0){
      list_concat(res, moves[i]);
      layers--;
    }
    list_free(moves[i]);
  }
  free(moves);
  list_cut(res, nb_moves);
  return res;
}

void matrix_clear(struct matrix * m, int value){
  for(size_t i =0; i < m->size * m->size; i++){
    m->m[i] = value;
  }
}

struct matrix * matrix_copy(struct matrix * m){
  struct matrix * m_new = malloc(sizeof(*m_new));
  m_new->size = m->size;
  m_new->m = malloc(sizeof(int) * m->size * m->size);
  for(size_t i = 0; i < m->size * m->size; i++){
    m_new->m[i] = m->m[i];
  }
  return m_new;
}

int matrix_sum(struct matrix * m){
  int res = 0;
  for(size_t i = 0; i < m->size * m->size; i++){
    res += m->m[i];
  }
  return res;
}

void matrix_number(struct matrix *m, int * s){
  for (size_t i = 0; i < m->size*m->size; i++){
    if (m->m[i] >= 0){
      s[m->m[i]]++;
    }
  }
}


void matrix_free(struct matrix * m){
  free(m->m);
  free(m);
}

void matrix_print(struct matrix *m){
  char tmp[3];
  for(int i = 0; i < m->size; i++){
    for(int j = 0; j < m->size; j++){
      if (m->m[i * m->size + j] < 0){
        sprintf(tmp, "+");
      }
      else{
        sprintf(tmp, "%d", m->m[i * m->size + j]);
      }
      printf("%s", tmp);
      for(int k = strlen(tmp); k < 4; k++){
        printf(" ");
      }
    }
    printf("\n");
  }
}
