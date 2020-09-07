#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../server/moves.h"
#include "../server/move.h"
#include "../server/bitboard.h"
#include "test_utils.h"

//install(TARGET main DESTINATION)
//in the Makefile -DCMAKE_INSTALL_PREFIX=../install ..
// (CMAKE_C_FLAGS "${CMAKE_C_FLAGS -std = c99}")

int test_initialize(){
  struct moves* m = moves__initialize();
  if(moves__get_size(m) != 0){
    moves__free(m);
    return EXIT_FAILURE;
  }
  if(moves__last_n(m)==NULL){
    moves__free(m);
    return EXIT_FAILURE;
  }
  moves__free(m);
  return EXIT_SUCCESS;
}

int test_add_opening(){
  struct board* board = board__initialize(SIZE);
  struct moves* m = moves__initialize();
  struct col_move_t * c = malloc(sizeof(struct col_move_t)*4);
  struct col_move_t c1 = {{1,0},BLACK};
  struct col_move_t c2 = {{2,0},WHITE};
  struct col_move_t c3 = {{3,0},BLACK};
  *(c) = c1;
  *(c+1) = c2;
  *(c+2) = c3;
  moves__add_opening(c,m,board);
  if(moves__get_size(m) != 3){
    moves__free(m);
    board__free(board);
    return EXIT_FAILURE;
  }
  if(moves__last_n(m)[1].m.row != c2.m.row){
    moves__free(m);
    board__free(board);
    return EXIT_FAILURE;
  }
  moves__free(m);
  board__free(board);
  return EXIT_SUCCESS;
}

int test_enqueue(){
  struct board* board = board__initialize(SIZE);
  struct moves * moves = moves__initialize();
  struct col_move_t c1 = {{1,0},BLACK};
  struct col_move_t c2 = {{2,0},WHITE};
  struct col_move_t c3 = {{3,0},BLACK};
  struct col_move_t c4 = {{4,0},WHITE};
  moves__enqueue(moves,&c1);
  if(moves__get_size(moves) != 1 && moves__last_n(moves)[0].m.row != 1){
    moves__free(moves);
    return EXIT_FAILURE;
  }
  moves__enqueue(moves,&c2);
  if(moves__get_size(moves) != 2 && moves__last_n(moves)[1].m.row != 1 && moves__last_n(moves)[0].m.row != 2){
    moves__free(moves);
    board__free(board);
    return EXIT_FAILURE;
  }
  moves__free(moves);
  moves = moves__initialize();
  struct col_move_t * c = malloc(sizeof(struct col_move_t)*4);
  *(c) = c1;
  *(c+1) = c2;
  *(c+2) = c3;
  moves__add_opening(c,moves,board);
  moves__enqueue(moves,&c4);
  if(moves__get_size(moves) != 4 &&
     moves__last_n(moves)[1].m.row != 2 &&
     moves__last_n(moves)[2].m.row != 3 &&
     moves__last_n(moves)[3].m.row != 4){
    moves__free(moves);
    board__free(board);
    return EXIT_FAILURE;
  }
  moves__enqueue(moves, &c3);
  if(moves__get_size(moves) != 2 &&
     moves__last_n(moves)[0].m.row != 4 &&
     moves__last_n(moves)[1].m.row != 3){
    moves__free(moves);
    board__free(board);
    return EXIT_FAILURE;
  }
  moves__enqueue(moves,&c1);
  if(moves__get_size(moves) != 2 &&
     moves__last_n(moves)[0].m.row != 3 &&
     moves__last_n(moves)[1].m.row != 1){
    moves__free(moves);
    board__free(board);
    return EXIT_FAILURE;
  }
  moves__free(moves);
  board__free(board);
  return EXIT_SUCCESS;
}

// if you want, you can use the function "print_error" in test_player.c
// that I recently add and that make a lovely print ;)
int main(){
  long clk_tck = CLOCKS_PER_SEC;
  clock_t t1, t2;
  t1 = clock();
  int total = 0;
  int res = 0;
  printf("\n\nTEST FOR MOVES FUNCTIONS\n\n");
  res = test_initialize();
  total = total + res;
  printf("test_initialize() is %s\n",res?"FAILED":"SUCCESS");
  res = test_add_opening();
  total = total + res;
  printf("test_add_opening() is %s\n",res?"FAILED":"SUCCESS");
  res = test_enqueue();
  total = total + res;
  printf("test_enqueue() is %s\n",res?"FAILED":"SUCCESS");
  printf("NOMBRE D'ECHEC : %d\n",total);
  t2 = clock();
  printf("Elapsed time : %lf \n",(double)(t2-t1)/(double)clk_tck);
  return 0;
}
