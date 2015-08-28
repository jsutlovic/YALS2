#include <limits.h>
#include <stdio.h>
#include "world.h"

static const char DISPLAY_CHARS[4] = { ' ', 'o', '*', 'O' };
/*
 * Number of set bits in the lowest 3 'even' bit positions.
 * Using a number masked by 0x2a as an index to this array,
 * the value of the array at that index is the number of bits set
 * in the number used to index.
 * e.g. BIT_COUNTS[0x2 & 0x2a] == 1, BIT_COUNTS[0x537 & 0x2a] == 2
 */
const char BIT_COUNTS[BIT_COUNT_LEN] = {
    0,   0,   1,   1,   0,   0,   1,   1,
    1,   1,   2,   2,   1,   1,   2,   2,
    0,   0,   1,   1,   0,   0,   1,   1,
    1,   1,   2,   2,   1,   1,   2,   2,
    1,   1,   2,   2,   1,   1,   2,   2,
    2,   2,   3,   3,   2,   2,   3,   3,
    1,   1,   2,   2,   1,   1,   2,   2,
    2,   2,   3,   3,   2,   2,   3,   3
};

world* init_world(size_t xlim, size_t ylim) {
    world *w = malloc(sizeof(world));
    w->xlim = xlim;
    w->ylim = ylim;
    w->generation = 0;
    w->state = CALC;

    // TODO: Check if xlim and ylim are >= sqrt(SIZE_MAX/2)

    w->data_size = ( xlim * ylim * 2.0 ) / ( sizeof(world_store) * CHAR_BIT ) + .969;

    w->data = calloc(w->data_size, sizeof(world_store));
    return w;
}

void destroy_world(world *w) {
    free(w->data);
    free(w);
}

void iter_world(world *w, iter_world_func_type itf) {
    size_t x = 0, y = 0;
    world_store cell_val, cell_mask;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_ELEM-1; j >= 0; j--) {
            cell_mask = SINGLE_CELL_MASK << j*BITS_PER_CELL;
            cell_val = (w->data[i] & cell_mask) >> j*BITS_PER_CELL;

            itf(w, x, y, &cell_val);
            w->data[i] = (w->data[i] & (~cell_mask)) | ((cell_val << j*BITS_PER_CELL) & cell_mask);

            x++;
            if (x >= w->xlim) {
                x = 0;
                y++;
                if (y >= w->ylim) {
                    break;
                }
            }
        }
    }
}

void _print_world_it(world *w, size_t x, size_t y, world_store *val) {
    size_t index = w->state ? *val : (*val & 2) | (*val >> 1);
    putchar(DISPLAY_CHARS[index]);
    if (x == w->xlim-1) {
        putchar('\n');
    }
}

void print_world(world *w) {
    printf("World %lux%lu, state: %s, gen %lu:\n",
            w->xlim,
            w->ylim,
            w->state ? "SHIFT" : "CALC",
            w->generation);
    iter_world(w, _print_world_it);
}
