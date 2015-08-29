#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>
#include <stdlib.h>

#define BITS_PER_CELL 2
#define CELLS_PER_ELEM 16
#define CURR_CELL_MASK 0xaaaaaaaa
#define BIT_COUNT_LEN 64 // 2^6
#define NEXT_STATE_MASK 0x1
#define SINGLE_CELL_MASK 0x3
#define MULTI_CELL_MASK 0x3f

/*** TYPES ***/

typedef uint32_t world_store;

enum world_state { CALC=0, SHIFT=1 };
typedef enum world_state world_state;

typedef struct world world;
struct world {
    size_t xlim;
    size_t ylim;
    size_t data_size;
    unsigned long generation;
    world_state state;
    world_store *data;
};

typedef void (*iter_world_func_type) (world *w, size_t x, size_t y, world_store *cell_val);


extern const char BIT_COUNTS[];


/*** FUNCTIONS ***/

world* init_world(size_t xlim, size_t ylim);
void destroy_world(world *w);
void print_world(world *w);
void iter_world(world *w, iter_world_func_type itf);

#endif
/* vim: set ft=c : */
