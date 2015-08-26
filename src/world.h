#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>
#include <stdlib.h>

#define BITS_PER_CELL 2
#define CELLS_PER_VAL 16

typedef uint32_t world_store;

enum world_state { CALC=0, SHIFT=1 };
typedef enum world_state world_state;

typedef struct world world;
struct world {
    size_t length;
    size_t width;
    size_t data_size;
    unsigned long generation;
    world_state state;
    world_store *data;
};

typedef void (*iter_world_func_type) (world *w, size_t x, size_t y, world_store *cell_val);

world* init_world(size_t length, size_t width);
void destroy_world(world *w);
void print_world(world *w);
void iter_world(world *w, iter_world_func_type itf);

#endif
/* vim: set ft=c : */
