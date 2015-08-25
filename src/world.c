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

void print_world(world *w) {
    printf("World %lux%lu, gen %lu:\n", w->world_length, w->world_width, w->generation);

    size_t cell_index, cell_row_count = 0, cell_col_count = 0;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_VAL-1; j >= 0; j--) {
            cell_index = (w->data[i] & (0x3 << j*BITS_PER_CELL)) >> j*BITS_PER_CELL;
            putchar(DISPLAY_CHARS[cell_index]);
            cell_col_count++;
            if (cell_col_count >= w->world_width) {
                putchar('\n');
                cell_col_count = 0;
                cell_row_count++;
                if (cell_row_count >= w->world_length) {
                    break;
                }
            }
        }
    }
}
