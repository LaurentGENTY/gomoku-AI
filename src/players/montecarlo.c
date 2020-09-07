#include "stdio.h"
#include "stdlib.h"
#include "montecarlo.h"
#include "math.h"
#include "list.h"
#include "time.h"
#include "../server/board.h"
#include <pthread.h>

extern struct player self;

struct tree generate_tree(){
    struct tree t;
    struct move_t mv = {-1,-1}; // the root is not a real move
    t.root = generate_node(NULL, mv);
    t.tmp = board__initialize(self.size);
    return t;
}

struct node * generate_node(struct node * father, struct move_t mv){
    struct node * n = malloc(sizeof(struct node));
    n->nb_simulations = 0.0;
    n->nb_wins = 0.0;
    n->nb_childs = 0;
    n->childs = NULL;
    n->father = father;
    n->game_over = 0;
    n->mv = mv;
    return n;
}

int node_is_leaf(struct node * n){
    return n->nb_childs == 0;
}

void node_add_child(struct node * n, struct node * child){
    //printf("adding %ld %ld to %ld %ld\n", child->mv.row, child->mv.col, n->mv.row, n->mv.col);
    n->nb_childs++;
    n->childs = realloc(n->childs, sizeof(struct node*) * n->nb_childs);
    n->childs[n->nb_childs-1] = child;
}

void free_tree(struct tree * t){
    free_node(t->root);
    board__free(t->tmp);
}

void free_node(struct node * n){
    for(int i = 0; i < n->nb_childs; i++){
        free_node(n->childs[i]);
    }
    if (n->childs != NULL){
        free(n->childs);
    }
    free(n);
}

int equal_moves(struct move_t m1, struct move_t m2){
    return m1.row == m2.row && m1.col == m2.col;
}

struct move_t get_better_move(struct tree * t){
    if(t->root->nb_childs == 0){printf("error !\n");}
    struct node * n = t->root->childs[0];
    for(int i = 0; i < t->root->nb_childs; i++){
        /*printf("%ld %ld is %f %f %d\n", t->root->childs[i]->mv.row,
                                        t->root->childs[i]->mv.col,
                                        t->root->childs[i]->nb_wins,
                                        t->root->childs[i]->nb_simulations,
                                        t->root->childs[i]->game_over);*/
        if ((n->game_over && t->root->childs[i]->game_over) || (!n->game_over && !t->root->childs[i]->game_over)){
            if (t->root->childs[i]->nb_simulations > n->nb_simulations){
                n = t->root->childs[i];
            }
            else if (t->root->childs[i]->nb_simulations == n->nb_simulations){
                if(t->root->childs[i]->nb_wins > n->nb_wins){
                    n = t->root->childs[i];
                }
            }
        }
        else if (t->root->childs[i]->game_over){
            n = t->root->childs[i];
        }
    }
    //printf("%f %f\n", n->nb_wins, n->nb_simulations);
    return n->mv;
}


void relocate_root(struct tree *t, struct col_move_t mv){
    struct node * n = NULL;
    for(int i = 0; i < t->root->nb_childs; i++){
        if (!equal_moves(t->root->childs[i]->mv, mv.m)){
            free_node(t->root->childs[i]);
        }
        else{
            n = t->root->childs[i];
        }
    }
    free(t->root->childs);
    free(t->root);
    if (n != NULL){
        t->root = n;
        n->father = NULL;
        //printf("relocation\n");
    }
    else{
        //printf("regenerate the tree\n");
        *t = generate_tree();
    }
}

// =============================================================================

double value_of_node(struct node * n){
    if (n->nb_simulations != 0){
        if (n->father != NULL){
            return (n->nb_wins / n->nb_simulations) + sqrt(2 * log(n->father->nb_simulations / n->nb_simulations));
        }
    }
    //printf("error value\n");
    return 0.0;
}

struct node * selection(struct tree * t, int color){
    //printf("selection %ld %ld\n", t->root->mv.row, t->root->mv.col);
    // browse the childs
    // choose with the formula exploration/exploitation
    // recursion on the chosen child
    board__free(t->tmp);
    t->tmp = board__copy(self.board);
    t->nb_moves = self.nb_moves;
    struct node * n = t->root;
    struct node * next_n = t->root;
    while(!node_is_leaf(next_n)){
        double value = -1.0;
        for(int i = 0; i < n->nb_childs; i++){
            double current_value = value_of_node(n->childs[i]);
            //printf("node %ld %ld is valued %f\n", n->childs[i]->mv.row, n->childs[i]->mv.col, current_value);
            if (current_value > value){
                value = current_value;
                next_n = n->childs[i];
            }
        }
        n = next_n;
        //printf("choose %ld %ld\n", n->mv.row, n->mv.col);
        board__add_move(t->tmp, next_n->mv, color);
        color = (color+1)%2;
        t->nb_moves++;
        //printf("on descend a %ld %ld\n", next_n->mv.row, next_n->mv.col);
    }
    t->color_for_expansion = color; //color of the new nodes
    return n;
}

struct data{ // for the threads
    int result;
    struct tree * t;
    struct node * n;
};

static void * thread_simulation(void * data);

void expansion(struct tree * t, struct node * n){
    //printf("expansion\n");
    // add all potential moves
    struct list * moves = board__possible_moves(t->tmp);
    if (!equal_moves(n->mv, t->root->mv) && board__won(t->tmp, n->mv)){
        n->game_over = 1;
        int a = t->color_for_expansion == (self.color+1)%2 && t->nb_moves < self.size*self.size;
        if(1){printf("victory !! %ld %ld %d %d\n", n->mv.row, n->mv.col, a, n->game_over);}
        for(int i = 0; i < list_length(moves); i++){
            update(n, a);
        }
    }
    else{
        struct data * dati = malloc(sizeof(struct data) * list_length(moves));
        pthread_t * threads = malloc(sizeof(pthread_t)*list_length(moves));
        for(int i = 0; i < list_length(moves); i++){
            struct move_t mv = list_get(moves, i);
            struct node * new_n = generate_node(n, mv);
            node_add_child(n, new_n);
            dati[i].t = t;
            dati[i].n = new_n;
            dati[i].result = 0;
            //int a = simulation(t, new_n);
            pthread_create(threads+i, NULL, thread_simulation, &dati[i]);
        }
        for(int i = 0; i < list_length(moves); i++){
            pthread_join (threads[i], NULL);
            update(n->childs[i], dati[i].result);
            //printf("adding %ld %ld with value %d\n",n->childs[i]->mv.row, n->childs[i]->mv.col,dati[i].result);
        }
        free(dati);
        free(threads);
    }
    list_free(moves);
}

// return if we have won
int simulation(struct tree * t, struct node * n){
    //printf("simulate on %ld %ld\n",n->mv.row, n->mv.col);
    // simulate this move
    struct board * b = board__copy(t->tmp);
    int nb_moves = t->nb_moves + 1;
    board__add_move(b, n->mv, t->color_for_expansion);
    // play randomly until victory
    int color = t->color_for_expansion;
    struct move_t mv = n->mv;
    while(!board__won(b, mv) && nb_moves < self.size*self.size){
        mv = board__possible_move(b);
        color = (color+1)%2;
        board__add_move(b, mv, color);
        nb_moves++;
    }
    if (nb_moves == t->nb_moves + 1){
        n->game_over = 1;
        //printf("feuille\n");
    }
    board__free(b);
    return color == self.color && nb_moves < self.size*self.size;
}

void update(struct node * n, int simulated_res){
    while(n != NULL){
        //printf("update ");
        n->nb_wins += simulated_res;
        n->nb_simulations++;
        n = n->father;
    }
}

static void * thread_simulation(void * data){
    struct data * d = (struct data *)data;
    d->result = simulation(d->t, d->n);
    return NULL;
}
