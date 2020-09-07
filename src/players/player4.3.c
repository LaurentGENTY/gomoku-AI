/**
 * @file player4.3.c
 * This player implements the heuristic2.h and play with a minmax strategy with a maximal depth and a number of childs maximum. Threads are also used in order to speed computing
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "player.h"
#include "../server/move.h"
#include "matrix.h"
#include "heuristic2.h"

/**
 * @brief Struct for containing data by the player
 */
struct player{
  /**
   * own heuristic to evaluate board
   */
  struct heuristic * h;
  /**
   * size of the board
   */
  size_t size;
  /**
   * id of the player
   */
  int color;
  /**
   * current number of moves
   */
  int nb_moves;
};

/* Struct for containing data by the player */
struct player self;

/**
 * @brief Informations for a thread
 */
struct data{
  /**
   * move
   */
  struct move_t mv;
  /**
   * current step
   */
  int step;
  /**
   * copy of the heuristic
   */
  struct heuristic * cp;
};

/* Internal functions */

/**
 * @brief Evaluate all data in parallel with the heuristic of the player
 * @param[in] data all data for a thread
 */
static void * thread_evaluation(void * data){
  struct data * m = (struct data*)data;
  heuristic_actualize(m->cp, m->mv, self.color);
  m->step = -negamax(m->cp, m->cp->depth, -INFINITY, INFINITY, (self.color+1)%2);
  return NULL;
}

/* External functions */

char const* get_player_name(){
  return "C warrior 4.3";
}

struct col_move_t* propose_opening(size_t size){
  struct col_move_t cm;
  initialize(size,BLACK);
  cm.m.row = size / 2;
  cm.m.col =  size / 2;
  cm.c = self.color;
  struct col_move_t * cmv = malloc(sizeof(struct col_move_t)*3);
  *cmv = cm;
  struct col_move_t cm1;
  struct col_move_t cm2;
  self.color = WHITE;
  cm1.m = play(cmv,1);
  cm1.c = self.color;
  *(cmv+1) = cm1;
  self.color = BLACK;
  cm2.m = play(cmv,2);
  cm2.c = BLACK;
  *(cmv+2) = cm2;
  self.nb_moves = 0;
  finalize();
  return cmv;
}

int accept_opening(size_t size, const struct col_move_t* opening){
  initialize(size,WHITE);
  struct move_t m = play(opening,3);
  heuristic_actualize(self.h, m, self.color);
  int h = heuristic_eval_game(self.h, self.color);
  struct col_move_t cmv1 = {m,WHITE};
  struct col_move_t* open = malloc(sizeof(struct col_move_t)*4);
  *open = *opening ;
  *(open + 1) = *(opening + 1);
  *(open + 2) = *(opening + 2);
  *(open + 3) = cmv1;
  self.color = BLACK;
  struct move_t m1 = play(open,4);
  heuristic_actualize(self.h, m1, self.color);
  int h1 = heuristic_eval_game(self.h, self.color);
  heuristic_remove(self.h,m);
  heuristic_remove(self.h,m1);
  self.nb_moves = 0;
  free(open);
  finalize();
  if(h1 < h){
    return 0;
  }
  return 1;
}

void initialize(size_t size, enum color_t id){
  self.color = id;
  self.size = size;
  self.nb_moves = 0;
  self.h = heuristic_init(size, id, 5, 6, 3); // size, color, depth, childs, layer
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  // updating actual board
  for(size_t i = 0; i < n_moves; i++){
    heuristic_actualize(self.h, previous_moves[i].m, previous_moves[i].c);
    self.nb_moves++;
  }
  if (self.nb_moves == 0){
    struct move_t mv = {self.size/2, self.size/2};
    return mv;
  }

  // applying min_max
  struct move_t next_mv;
  printf("===============================\n");
    matrix_print(self.h->opportunity);
    printf("===============================\n");
    matrix_print(self.h->danger);
    printf("===============================\n");

  self.h->depth = 4;
  self.h->nb_childs = 6;
  if (self.nb_moves < 20){ self.h->depth = 4; self.h->nb_childs = 8; }
  if (self.nb_moves < 6){ self.h->depth = 4; self.h->nb_childs = 10; }

  //self.h->depth += 2;

  long clk_tck = CLOCKS_PER_SEC;
  clock_t t1, t2;
  t1 = clock();
  int nb_nodes = 0;
  self.h->nb_node_explored = 0;

  int better_move = -INFINITY;
  struct list * moves = generating_moves(self.h, self.color);
  struct move_t m = list_get(moves, 0);
  int my_force = heuristic_potential_point(self.h, m);
  if (my_force >= MAX_VAL){
    list_free(moves);
    return m;
  }

  if (list_length(moves) == 0){printf("error !\n");}
  //printf("nombre de coups consideres : %d\n", list_length(&moves));

  // multithreading
  // we need to make a copy of the heuristic because its value will be modified
  // during recursion
  pthread_t * threads = malloc(sizeof(pthread_t)*list_length(moves));
  struct data * dati = malloc(sizeof(struct data)*list_length(moves));
  for (int i = 0; i < list_length(moves); i++){
    dati[i].mv = list_get(moves, i);
    dati[i].step = 0;
    dati[i].cp = heuristic_copy(self.h);
  }
  for(int i = 0; i < list_length(moves); i++){
    pthread_create(threads+i, NULL, thread_evaluation, &dati[i]);
  }
  for(int i = 0; i < list_length(moves); i++){
    pthread_join (threads[i], NULL);
    //printf("power of %ld %ld : %d\n", dati[i].mv.row, dati[i].mv.col, dati[i].step);
    if (dati[i].step > better_move){
      better_move = dati[i].step;
      next_mv = list_get(moves, i);
    }
    nb_nodes += dati[i].cp->nb_node_explored;
    heuristic_free(dati[i].cp);
  }
  t2 = clock();
  printf("%d noeuds en %lf secondes\n", nb_nodes, (double)(t2-t1)/(double)clk_tck);
  free(threads);
  free(dati);
  list_free(moves);
  return next_mv;
}

void finalize(){
  heuristic_free(self.h);
}
