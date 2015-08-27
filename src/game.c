#include <stdio.h>
#include "game.h"

game* init_game(size_t length, size_t width, rule_calc_func_type rule) {
    game *g = malloc(sizeof(game));
    g->rule = rule;
    g->w = init_world(length, width);
    return g;
}

void destroy_game(game *g) {
    destroy_world(g->w);
    free(g);
}

void _calc_next_state(game *g) {
    size_t x = 0, y = 0;
    world *w = g->w;
    world_store cell_mask, cell_count_val;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_ELEM-1; j >= 0; j--) {
            /* printf("i: %2lu, j: %2d, x: %2lu, y: %2lu, ", i, j, x, y); */
            char cell_count = 0;

            if (j == CELLS_PER_ELEM-1 && i > 0) {
                /* putchar('1'); */
                // First cell in element
                cell_mask = MULTI_CELL_MASK << (j-1)*BITS_PER_CELL;
                cell_count_val =
                    ((w->data[i] & cell_mask) >> (j-2)*BITS_PER_CELL) |
                    (w->data[i-1] & 0x3);
            } else if (j == 0) {
                /* putchar('2'); */
                // Last cell in element
                cell_mask = MULTI_CELL_MASK >> BITS_PER_CELL;
                cell_count_val = w->data[i] & cell_mask;
                if (i < w->data_size-1) {
                    int high_bits = (CELLS_PER_ELEM-1)*BITS_PER_CELL;
                    cell_count_val =
                        cell_count_val |
                        ((w->data[i+1] & (0x3 << high_bits)) >> high_bits);
                }
            } else {
                /* putchar('3'); */
                cell_mask = MULTI_CELL_MASK << (j-1)*BITS_PER_CELL;
                cell_count_val = (w->data[i] & cell_mask) >> (j-1)*BITS_PER_CELL;
                if (x == 0) {
                    cell_count_val &= 0xf;
                } else if (x == w->width) {
                    cell_count_val &= 0x3c;
                }
            }

            cell_count += BIT_COUNTS[cell_count_val];

            /* printf(" %2x %d ", cell_count_val, cell_count); */
            printf("%d ", cell_count);
            /* printf(" %x ", w->data[i]); */

            x++;
            if (x >= w->length) {
                putchar('\n');
                x = 0;
                y++;
                if (y >= w->width) {
                    break;
                }
            }
        }
    }
}

void game_half_step(game *g) {
    switch (g->w->state) {
        case CALC:
            _calc_next_state(g);
            break;
        case SHIFT:
            break;
    }
}

void game_step(game *g) {
    game_half_step(g);
}
