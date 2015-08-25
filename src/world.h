#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>
#include <stdlib.h>

#define BITS_PER_CELL 2
#define CELLS_PER_VAL 16

typedef struct world world;
struct world {
    size_t world_length;
    size_t world_width;
    size_t data_size;
    unsigned long generation;
    uint32_t *data;
};

world* init_world(size_t length, size_t width);
void destroy_world(world *w);
void print_world(world *w);

#endif
/* vim: set ft=c : */
