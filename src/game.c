#include <stdio.h>
#include "game.h"

#define DEBUG 0
#define DEBUG2 0

game* init_game(size_t xlim, size_t ylim, rule_calc_func_type rule) {
    game *g = malloc(sizeof(game));
    g->rule = rule;
    g->w = init_world(xlim, ylim);
    return g;
}

void destroy_game(game *g) {
    destroy_world(g->w);
    free(g);
}

void _calc_next_state(game *g) {
    size_t x = 0, y = 0;
    world *w = g->w;
    world_store cell_val, cell_mask, cell_count_val;
    size_t ci;
    int cj;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_ELEM-1; j >= 0; j--) {
#if DEBUG
            printf("i: %2lu, j: %2d, x: %2lu, y: %2lu, ", i, j, x, y);
#endif
            char cell_count = 0;

            // Get surrounding counts for previous, current and next rows
            for (int dd = -1; dd < 2; dd++) {
                if ((y == 0 && dd == -1) || (y == w->ylim-1 && dd == 1)) {
                    continue;
                }
                // Current element index (i) and index-in-element (j)
                ci = ((y+dd)*w->xlim + x) / CELLS_PER_ELEM;
                cj = CELLS_PER_ELEM - 1 - (((y+dd)*w->xlim + x) % CELLS_PER_ELEM);

#if DEBUG
                printf("|%2d ci: %2lu, cj: %2d, (", dd, ci, cj);
#endif

                if (cj == CELLS_PER_ELEM-1 && ci > 0) {
                    // First cell in element
                    cell_mask = MULTI_CELL_MASK << (cj-1)*BITS_PER_CELL;
#if DEBUG
                    putchar('1');
                    printf(" m: %08x ", cell_mask);
#endif
                    // Get first 2 cells in current element
                    // Add last cell of previous element
                    cell_count_val =
                        ((w->data[ci] & cell_mask) >> (cj-1)*BITS_PER_CELL) |
                        ((w->data[ci-1] & 0x3) << 2*BITS_PER_CELL);
                } else if (cj == 0) {
                    // Last cell in element
                    cell_mask = MULTI_CELL_MASK >> BITS_PER_CELL;
#if DEBUG
                    putchar('2');
                    printf(" m: %08x ", cell_mask);
#endif
                    // Get last 2 cells in current element
                    // Get first cell of next element
                    cell_count_val = (w->data[ci] & cell_mask) << BITS_PER_CELL;
                    if (ci < w->data_size-1) {
                        int high_bits = (CELLS_PER_ELEM-1)*BITS_PER_CELL;
                        cell_count_val |=
                            ((w->data[ci+1] & (0x3 << high_bits)) >> high_bits);
                    }
                } else {
                    // Get the surrounding 2 cells
                    cell_mask = MULTI_CELL_MASK << (cj-1)*BITS_PER_CELL;
                    cell_count_val = (w->data[ci] & cell_mask) >> (cj-1)*BITS_PER_CELL;
#if DEBUG
                    putchar('3');
                    printf(" m: %08x ", cell_mask);
#endif
                }

                // Don't take the left cell if the row has started
                // Don't take the right cell if the row has ended
                if (x == 0) {
                    cell_count_val &= 0xf;
                } else if (x == w->xlim-1) {
                    cell_count_val &= 0x3c;
                }

                cell_count += BIT_COUNTS[cell_count_val];

#if DEBUG
                printf(") %2x %d ", cell_count_val, cell_count);
#endif
            }

#if DEBUG
            printf(" %x", w->data[i]);
            putchar('\n');
#endif

            cell_mask = SINGLE_CELL_MASK << j*BITS_PER_CELL;
            cell_val = (w->data[i] & cell_mask) >> j*BITS_PER_CELL;
            cell_val =
                (cell_val & 2) |
                (g->rule(cell_val, cell_count) & 1) &
                SINGLE_CELL_MASK;
            w->data[i] =
                (w->data[i] & (~cell_mask)) |
                (cell_val << j*BITS_PER_CELL);

            x++;
            if (x >= w->xlim) {
#if DEBUG || DEBUG2
                putchar('\n');
#endif
                x = 0;
                y++;
                if (y >= w->ylim) {
                    break;
                }
            }
        }
    }

    g->w->state = SHIFT;
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
