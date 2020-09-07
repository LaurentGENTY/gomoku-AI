#include "stdio.h"
#include "string.h"
#include "time.h"
#include "../players/heuristic2.h"
#include "../players/player.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int print_error(int e, char * str){
    printf("%s ", str);
    for(int i = 0; i < 34 - strlen(str);i++){
        printf(".");
    }
    if (e){ printf(RED " FAILED" RESET "\n"); }
    else{ printf(GREEN " SUCCESS" RESET "\n"); }
    return e;
}

/*
 * tests for the heuristic
 * -> evaluate_point        ==> verify the returned value
 * -> generating_moves    ==> verifiy that the moves are the most valuable
 * -> heuristic_actualize ==> verify the values in h->potential
 * -> heuristic_remove    ==> verify that the move is valid again
 * -> heuristic_full      ==> ...
 */

int test_explore_lines(){
    struct heuristic * h = heuristic_init(8,1,0,0,0);
    struct move_t moves[] = {{1,2},{2,2},{1,4},{2,3},{2,1},{3,2},{2,4},{3,5},{2,6},{3,6},
                                {3,1},{4,1},{3,3},{4,2},{3,4},{4,3},{4,0},{4,4},{4,5},{4,6},
                                {5,2},{5,0},{5,6},{5,3},{6,2},{5,4},{6,5}};
    for(int i = 0; i < 27; i++){
        heuristic_actualize(h, moves[i], i%2);
    }
    //matrix_print(h->opportunity);
    struct move_t mv[] = {{5,7},{7,2},{7,3},{7,4}};
    int rep[] = {1,1,1,1};
    for(int i = 0; i < 4; i++){
        int a = matrix_get(h->opportunity, mv[i].row, mv[i].col);
        if (a != rep[i]){
            printf("expected %d at %ld %ld, got %d\n", rep[i], mv[i].row, mv[i].col, a);
            heuristic_free(h);
            return 1; //error
        }
    }
    heuristic_free(h);
    return 0; //SUCCESS
}

int test_generating_moves(){
    struct heuristic * h = heuristic_init(8, 0,0,6,3);
    struct move_t moves[] = {{1,2},{2,2},{1,4},{2,3},{2,1},{3,2},{2,4},{3,5},{2,6},{3,6},
                                {3,1},{4,1},{3,3},{4,2},{3,4},{4,3},{4,0},{4,4},{4,5},{4,6},
                                {5,2},{5,0},{5,6},{5,3},{6,2},{5,4},{6,5}};
    for(int i = 0; i < 27; i++){
        heuristic_actualize(h, moves[i], i%2);
    }
    struct list * l = generating_moves(h, 0);

    struct move_t expected[] = {{1,1},{1,3},{1,5},{6,1},{6,3},{6,4}};
    for(int i = 0; i < 6; i++){
      if (!(list_find(l, expected[i]))){
        printf("error : %ld %ld not found\n", expected[i].row, expected[i].col);
        list_free(l);
        heuristic_free(h);
        return 1;
      }
    }
    if (list_length(l) != 12){
        printf("error in length : %d\n", list_length(l));
        list_free(l);
        heuristic_free(h);
        return 1;
    }
    list_free(l);
    heuristic_free(h);
    return 0;
}

void heuristic_value(){
    struct heuristic * h = heuristic_init(10,0,0,0,0);
    struct move_t moves[] = {{1,2},{3,2},{1,3},{3,3},{1,4},{3,4},{1,5},{3,5}};
    for(int i = 0; i < 8; i++){
        heuristic_actualize(h, moves[i], i%2);
    }
    printf("heuristic value : %d\n", heuristic_eval_game(h,0));
    matrix_print(h->opportunity);
    printf("=================================\n");
    matrix_print(h->danger);
}

int test_heuristic_full(){
    struct heuristic * h = heuristic_init(9, 0,0,0,0);
    for (int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            struct move_t mv = {i,j};
            heuristic_actualize(h, mv, i%2);
        }
    }
    // the board should be full
    if (!heuristic_full(h)){
        heuristic_free(h);
        return 1;
    }
    heuristic_free(h);
    return 0;
}


int main(){
  long clk_tck = CLOCKS_PER_SEC;
  clock_t t1, t2;
  t1 = clock();
  print_error(test_explore_lines(), "test evaluate_point");
  print_error(test_generating_moves(), "test generating_moves");
  print_error(test_heuristic_full(), "test heuristic_full");
  t2 = clock();
  printf("Elapsed time : %lf \n",(double)(t2-t1)/(double)clk_tck);
  heuristic_value();
  return 0;
}
