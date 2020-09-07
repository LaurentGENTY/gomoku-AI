#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "../server/bitboard.h"
#include "../server/move.h"
#include "test_utils.h"

//Select the value of the bit of the board in the position
extern __uint128_t board__select_bit(__uint128_t* b,int pos);

int test_board__initialize(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  int err_w = 0;
  int err_b = 0;
  for(size_t i = 1; i < SIZE*SIZE+1; i ++){
      if(board__select_bit(board->b_w,i)!=0){
	err_w+=1;
      }
      if(board__select_bit(board->b_b,i)!=0){
	err_b+=1;
      }
  }
  for(size_t i = SIZE*SIZE+1; i < 129; i ++){
    if(board__select_bit(board->b_w,i)!=1){
      err_w+=1;
    }
    if(board__select_bit(board->b_b,i)!=1){
	err_b+=1;
      }
  }
  // Test if each case of the white board has been initialized
  TEST("initialization of the cases of the white board",\
       !err_w,act);
  // Test if each case of the black board has been initialized
  TEST("initialization of the cases of the white board",\
       !err_b,act);
  // Test if the capacity has been initialized
  TEST("initialization of the capacity of the board",\
       (board->capacity==SIZE*SIZE),act);
  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__is_full(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  *(board->b_w)=-1-7;
  *(board->b_b)=1;
  // Test if the board is not full
  TEST("board is not full",\
       !board__is_full(board),act);
  *(board->b_b)+=6;
  // Test if the board is full
  TEST("board is full",\
       board__is_full(board),act);

  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__is_valid_move(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move1= {-1,-1};
  struct move_t move2= {3,2};
  struct move_t move3= {0,0};
  struct move_t move4= {9,9};
  // Test if a non valide move isn't added in the board
  TEST("a non valid move don't change the board",\
       (!(board__is_valid_move(board,move1))),act);
  // Test if a valide black move is added only in the black board
  board__add_move(board,move2,BLACK);
  TEST("a valid black move change only the black board",
       ((board__select_bit(board->b_w,SIZE*move2.row+move2.col+1)==0)&&
	(board__select_bit(board->b_b,SIZE*move2.row+move2.col+1)==1)),act);
  // Test if when a move is played, he become invalid
  TEST("same move isn't valid",\
       (!(board__is_valid_move(board,move2))),act);
  // Test if a valide white move is added only in the white board
  // and didn't change the black board
  board__add_move(board,move3,WHITE);
  TEST("a valid white move change only the white board",		\
       ((board__select_bit(board->b_w,SIZE*move3.row+move3.col+1)==1)&&	\
	(board__select_bit(board->b_b,SIZE*move3.row+move3.col+1)==0)),act);
  // Test if a valide move don't erase the previous moves
  board__add_move(board,move4,BLACK);
  TEST("a valid move don't erase the previous moves",			\
       ((board__select_bit(board->b_b,SIZE*move4.row+move4.col+1)==1)&&		\
	(board__select_bit(board->b_b,SIZE*move2.row+move2.col+1)==1)),act);
  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__won(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move= {-1,-1};
  // Test if a empty board wins
  TEST("empty board doesn't win",\
       !board__won(board,move),act);
  board__free(board);

  enum winning_lane { ROW=0, COL=1, LDIAG=2, RDIAG=3 };
  int succ[] = {0,0,0,0}; // ROW,COL,LDIAG,RDIAG
  for(int i = 0;i<SIZE;i++){
    for(int j = 0;j<SIZE-NB_WIN;j++){

      // Generate every winning row
      board = board__initialize(SIZE);
      for(int k = j;k<j+NB_WIN;k++){
	move.row=i;move.col=k;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,BLACK);
      }
      succ[ROW]+=board__won(board,move); // Test if the current winning row wins
      board__free(board);
      // Generate every winning column

      board = board__initialize(SIZE);
      for(int k = j;k<j+NB_WIN;k++){
	move.row=k;move.col=i;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,WHITE);
      }
      succ[COL]+=board__won(board,move); // Test if the current winning column wins
      board__free(board);
    }
  }
  // Test if a board with a winning row wins
  TEST("winning row board wins",		\
       (succ[ROW]==(SIZE-NB_WIN)*SIZE),act);
  // Test if a board with a winning row wins
  TEST("winning column board wins",		\
       (succ[COL]==(SIZE-NB_WIN)*SIZE),act);
  for(int i = NB_WIN-1;i<SIZE;i++){
    for(int j = NB_WIN-1;j<SIZE;j++){
      // Generate every winning North-West diagonal
      board = board__initialize(SIZE);
      for(int k = 0;k<NB_WIN;k++){
	move.row=i-k;move.col=j-k;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,BLACK);
      }
      succ[LDIAG]+=board__won(board,move); // Test if the current winning
                               // North-West diagonal wins
      board__free(board);
    }
  }
  // Test if a board with a winning North-West diagonal wins
  TEST("winning North-West diagonal board wins",		\
       (succ[LDIAG]==(SIZE-NB_WIN+1)*(SIZE-NB_WIN+1)),act);

   for(int i = NB_WIN-1;i<SIZE;i++){
     for(int j = 0;j<SIZE-NB_WIN+1;j++){
       // Generate every winning column
       board = board__initialize(SIZE);
       for(int k = 0;k<NB_WIN;k++){
	 move.row=i-k;move.col=j+k;
	 (assert(board__is_valid_move(board,move)));
	 board__add_move(board,move,WHITE);
       }
       succ[RDIAG]+=board__won(board,move); // Test if the current winning
                                // North-East diagonal wins
       board__free(board);
     }
   }
   // Test if a board with a winning North-West diagonal wins
   TEST("winning North-East diagonal board wins",	\
	(succ[RDIAG]==(SIZE-NB_WIN+1)*(SIZE-NB_WIN+1)),act);
   END_TEST("-",act);
   //printf("%d %d %d %d\n",succ[ROW],succ[COL],succ[LDIAG],succ[RDIAG]);
   assert(res!=0);
   return (res==tot);
}

int test_explore__line(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move = {0,0};
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  move.row=0;move.col=1;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  int pattern[8] = {0,0,0,0,0,0,0,0};
  board__explore_line(board,0,2,2,BLACK,pattern);
  int test[8] = {0,0,0,0,1,0,0,0};
  TEST("1",
       (pattern!=test),act);
  board__explore_line(board,0,2,0,BLACK,pattern);
  test[7]+=1;
  TEST("2",
       (pattern!=test),act);
  move.row=0;move.col=3;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  board__explore_line(board,0,2,2,BLACK,pattern);
  test[2]+=1;
  TEST("3",
       (pattern!=test),act);
  move.row=0;move.col=4;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,WHITE);
  board__explore_line(board,0,2,2,BLACK,pattern);
  test[7]+=1;
  TEST("4",
       (pattern!=test),act);
  move.row=4;move.col=2;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,WHITE);
  board__explore_line(board,0,2,0,BLACK,pattern);
  test[7]+=1;
  TEST("5",
       (pattern!=test),act);
  move.row=1;move.col=1;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  board__explore_line(board,2,2,3,BLACK,pattern);
  test[4]+=1;
  TEST("6",
       (pattern!=test),act);
  move.row=3;move.col=3;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  board__explore_line(board,2,2,3,BLACK,pattern);
  test[2]+=1;
  TEST("7",
       (pattern!=test),act);
  move.row=4;move.col=4;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  board__explore_line(board,2,2,3,BLACK,pattern);
  test[0]+=1;
  TEST("8",
       (pattern!=test),act);
  move.row=5;move.col=5;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,WHITE);
  board__explore_line(board,2,2,3,BLACK,pattern);
  test[0]+=1;
  TEST("9",
       (pattern!=test),act);
  move.row=8;move.col=9;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,WHITE);
  board__explore_line(board,9,8,1,WHITE,pattern);
  test[7]+=1;
  TEST("10",
       (pattern!=test),act);
  move.row=6;move.col=4;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,WHITE);
  board__explore_line(board,5,3,3,WHITE,pattern);
  test[3]+=1;
  TEST("11",
       (pattern!=test),act);
  board__free(board);

  /* New tests*/

  board = board__initialize(SIZE-1);
  struct move_t moves[] = {{3,4},{3,5}};
  for(int i = 0; i < 2; i++){
    board__add_move(board, moves[i], i%2);
  }
  board__explore_line(board,4,3,1,BLACK,pattern);
  test[5]+=1;
  TEST("12",
       (pattern!=test),act);
  board__explore_line(board,3,3,2,BLACK,pattern);
  test[5]+=1;
  TEST("13",
       (pattern!=test),act);
  struct move_t moves2[] = {{2,4},{1,4},{2,5}};
  for(int i = 0; i < 3; i++){
    board__add_move(board, moves2[i], i%2);
  }
  board__explore_line(board,4,3,1,BLACK,pattern);
  test[5]+=1;
  TEST("14",
       (pattern!=test),act);
  board__explore_line(board,4,4,0,BLACK,pattern);
  test[5]+=1;
  TEST("15",
       (pattern!=test),act);
  struct move_t moves3[] = {{5,2},{2,2},{2,3}};
  for(int i = 0; i < 3; i++){
    board__add_move(board, moves3[i], i%2);
  }
  board__explore_line(board,4,3,1,BLACK,pattern);
  test[5]+=1;
  TEST("16",
       (pattern!=test),act);
  board__explore_line(board,2,6,2,BLACK,pattern);
  test[5]+=1;
  TEST("17",
       (pattern!=test),act);
  board__explore_line(board,7,1,2,BLACK,pattern);
  test[5]+=1;
  TEST("18",
       (pattern!=test),act);
  board__free(board);
  board = board__initialize(8);
  struct move_t moves4 = {3,3};
  board__add_move(board, moves4, BLACK);
  board__explore_line(board,7,3,3,BLACK,pattern);
  test[5]+=1;
  TEST("19",
       (pattern!=test),act);
  board__free(board);
  board = board__initialize(8);
  struct move_t moves5[] = {{1,3},{2,3},{3,3},{4,3}};
  for(int i =0;i<4;i++){
    board__add_move(board, moves5[i], BLACK);
  }
  struct move_t moves6 = {5,6};
  board__add_move(board, moves6, WHITE);
  board__explore_line(board,0,3,0,BLACK,pattern);
  test[5]+=1;
  TEST("20",
       (pattern!=test),act);
  board__free(board);
  board = board__initialize(8);
  struct move_t moves7[] = {{1,2},{1,4},{2,1},{2,4},{2,6},{3,1},{3,3},{3,4},{4,0},{4,5},{5,2},{5,6},{6,2},{6,5}};
  for(int i =0;i<14;i++){
    board__add_move(board, moves7[i], BLACK);
  }
  struct move_t moves8[] = {{2,2},{2,3},{3,2},{3,5},{3,6},{4,1},{4,2},{4,3},{4,4},{4,6},{5,0},{5,3},{5,4},{5,7}};
  for(int i =0;i<14;i++){
    board__add_move(board, moves8[i], WHITE);
  }
  int new_pattern[8] = {0,0,0,0,0,0,0,0};
  int new_test[8] = {0,0,0,0,0,0,0,0};
  //board__explore_line(board,5,7,0,WHITE,new_pattern);
  board__explore_line(board,5,7,1,WHITE,new_pattern);
  //board__explore_line(board,5,7,2,WHITE,new_pattern);
  //board__explore_line(board,5,7,3,WHITE,new_pattern);
  TEST("21",
       (new_pattern!=new_test),act);
  struct move_t move9 = {3,3};
  board__add_move(board, move9, BLACK);
  /*for(int m = 0;m<8;m++)
    printf("%d ",new_pattern[m]);
  printf("\n"); */
  //board__explore_line(board,7,3,0,BLACK,new_pattern);
  board__explore_line(board,7,3,1,BLACK,new_pattern);
  //board__explore_line(board,7,3,2,BLACK,new_pattern);
  //board__explore_line(board,7,3,3,BLACK,new_pattern);
  /*for(int m = 0;m<8;m++)
    printf("%d ",new_pattern[m]);
    printf("\n");*/
  struct move_t moves10[] = {{1,2},{2,2},{1,4},{2,3},{2,1},{3,2},{2,4},{3,5},{2,6},{3,6},
			   {3,1},{4,1},{3,3},{4,2},{3,4},{4,3},{4,0},{4,4},{4,5},{4,6},
			   {5,2},{5,0},{5,6},{5,3},{6,2},{5,4},{6,5}};
  for(int i = 0; i < 27; i++){
    board__add_move(board, moves10[i], i%2);
  }
  //board__explore_line(board,5,7,0,WHITE,new_pattern);
  board__explore_line(board,5,7,1,WHITE,new_pattern);
  //board__explore_line(board,5,7,2,WHITE,new_pattern);
  // board__explore_line(board,5,7,3,WHITE,new_pattern);
  /*for(int m = 0;m<8;m++)
    printf("%d ",new_pattern[m]);
    printf("\n");*/
  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int main(int argc,char* argv[]){
  //clock_t temps;
  //srand(time(NULL));
  FULL;
  INIT_TEST("*",1);
  TEST("initialization",test_board__initialize(act),1);
  TEST("board__is_full",test_board__is_full(act),1);
  TEST("board__is_valid_move",test_board__is_valid_move(act),1);
  //TEST("board__won",test_board__won(act),1);
  TEST("board__explore_line",test_explore__line(act),1);
  END_TEST("*",1);
  // printf("Bravo !\nTu as mis %f secondes a trouver le nombre.\n", (double) temps/CLOCKS_PER_SEC);
  return 0;
}
