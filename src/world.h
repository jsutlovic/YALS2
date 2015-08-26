#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>
#include <stdlib.h>

#define BITS_PER_CELL 2
#define CELLS_PER_VAL 16

typedef struct world world;
struct world {
    size_t length;
    size_t width;
    size_t data_size;
    unsigned long generation;
    uint32_t *data;
};

typedef void (*iter_world_func_type) (world *w, size_t x, size_t y, size_t *cell_val);

world* init_world(size_t length, size_t width);
void destroy_world(world *w);
void print_world(world *w);
void iter_world(world *w, iter_world_func_type itf);

#endif
/* vim: set ft=c : */
