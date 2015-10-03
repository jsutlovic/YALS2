#ifndef _WORLD_H
#define _WORLD_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "serialization.h"

#define PROGRAM_NAME "YALS2"
#define MINSIZE 17

#define WORLD_STORE_TYPE uint32_t
#define BITS_PER_CELL 2
#define CELLS_PER_ELEM 16
#define IDX_DIV 4 // log2 CELLS_PER_ELEM
#define OFFSET_MASK 0xf // (1 << IDX_DIV) - 1
#define CURR_CELL_MASK 0xaaaaaaaa

#define BIT_COUNT_LEN 64 // 2^6
#define NEXT_STATE_MASK 0x1
#define SINGLE_CELL_MASK 0x3
#define MULTI_CELL_MASK 0x3f
#define START_ROW_MASK 0xf
#define END_ROW_MASK 0x3c

/*** TYPES ***/

typedef WORLD_STORE_TYPE world_store;

enum world_state { CALC=0, SHIFT=1 };
typedef enum world_state world_state;

struct world {
    uint32_t xlim;
    uint32_t ylim;
    size_t cell_count;
    size_t data_size;
    uint32_t generation;
    world_state state;
    world_store *data;
    world_store *temp_calc;
};
typedef struct world world;

struct world_cell_pos {
    world *w;
    size_t x;
    size_t y;
    world_store *cell_val;
};
typedef struct world_cell_pos world_cell_pos;

typedef void (*iter_world_func_type) (world_cell_pos *wcp);

/*** FUNCTIONS ***/

world *init_world(uint32_t xlim, uint32_t ylim);
void destroy_world(world *w);
void print_world(world *w);
void iter_world(world *w, iter_world_func_type itf);
void invert_cell(world_cell_pos *p);
world *deserialize_world(char *data, size_t len);
char *serialize_world(world *w, size_t *len);
world *read_from_file(const char *filename);
size_t write_to_file(const char *filename, world *w);

void world_half_step(world *w);
void world_step(world *w);

#endif
/* vim: set ft=c : */
