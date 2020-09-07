#include "stdio.h"
#include "string.h"
#include "../players/matrix.h"

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

/* test on matrix lib
 * -> set and get
 * -> set and get max
 * -> list_moves_max
 */

int test_matrix_set_get(){
  struct matrix * m = matrix_generate(5,0);
  matrix_set(m, 3, 3, 5);
  int a = matrix_get(m, 3, 3);
  if (a != 5){
    matrix_free(m);
    return 1; //error
  }
  matrix_free(m);
  return 0;
}

int test_matrix_get_max(){
  struct matrix * m = matrix_generate(5,0);
  matrix_set(m, 3, 3, 5);
  int a = matrix_max(m);
  if (a != 5){
    matrix_free(m);
    return 1; //error
  }
  matrix_free(m);
  return 0;
}

int test_matrix_list_max(){
  struct matrix * m = matrix_generate(5,0);
  matrix_set(m, 3, 3, 5);
  matrix_set(m, 3, 2, 3);
  matrix_set(m, 1, 3, -1);
  matrix_set(m, 4, 2, 5);
  matrix_set(m, 2, 2, 5);
  struct list * l = matrix_list_moves_max(m);

  if (list_length(l) != 3){
    list_free(l);
    matrix_free(m);
    return 1; //error
  }
  struct move_t expected[] = {{3,3},{4,2},{2,2}};
  for (int i = 0; i < 3; i++){
    if (!list_find(l, expected[i])){
      printf("error expected %ld %ld not found\n", expected[i].row, expected[i].col);
      list_free(l);
      matrix_free(m);
      return 1; //error
    }
  }
  list_free(l);
  matrix_free(m);
  return 0;
}


int main(){
  print_error(test_matrix_set_get(), "test matrix set get");
  print_error(test_matrix_get_max(), "test matrix get max");
  print_error(test_matrix_list_max(), "test matrix list max");
  return 0;
}
