#include <limits.h>
#include <stdio.h>
#include "world.h"

static const char DISPLAY_CHARS[4] = { ' ', 'o', '*', 'O' };

world* init_world(size_t length, size_t width) {
    world *w = malloc(sizeof(world));
    w->world_length = length;
    w->world_width = width;
    w->generation = 0;

    // TODO: Check if length and width are >= sqrt(SIZE_MAX/2)

    w->data_size = ( length * width * 2.0 ) / ( sizeof(*w->data) * CHAR_BIT ) + .969;

    w->data = calloc(w->data_size, sizeof(*w->data));
    return w;
}

void destroy_world(world *w) {
    free(w->data);
    free(w);
}

void iter_world(world *w, iter_world_func_type itf) {
    size_t x = 0, y = 0;
    char cell_val;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_VAL-1; j >= 0; j--) {
            cell_val = (w->data[i] & (0x3 << j*BITS_PER_CELL)) >> j*BITS_PER_CELL;

            itf(w, x, y, cell_val);

            x++;
            if (x >= w->world_length) {
                x = 0;
                y++;
                if (y >= w->world_width) {
                    break;
                }
            }
        }
    }
}

void _print_world_it(world *w, size_t x, size_t y, size_t val) {
    putchar(DISPLAY_CHARS[val]);
    if (x == w->world_length-1) {
        putchar('\n');
    }
}

void print_world(world *w) {
    printf("World %lux%lu, gen %lu:\n", w->world_length, w->world_width, w->generation);
    iter_world(w, _print_world_it);
}
