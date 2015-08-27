#include <stdio.h>
#include "game.h"

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
    world_store cell_mask, cell_count_val;
    int di, dj;
    size_t ci;
    int cj;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_ELEM-1; j >= 0; j--) {
            /* printf("i: %2lu, j: %2d, x: %2lu, y: %2lu, ", i, j, x, y); */
            char cell_count = 0;

            for (int dd = -1; dd < 2; dd++) {
                if ((y == 0 && dd == -1) || (y == w->ylim-1 && dd == 1)) {
                    continue;
                }
                di = dd * (w->xlim / CELLS_PER_ELEM);
                dj = dd * (w->xlim % CELLS_PER_ELEM);
                cj = (j - dj) % CELLS_PER_ELEM;
                ci = i + di - (j - dj) / CELLS_PER_ELEM;
                if (cj < 0) {
                    cj += CELLS_PER_ELEM;
                    ci += 1;
                }

                /* printf("| ci: %2lu, cj: %3d, ", ci, cj); */

                if (cj == CELLS_PER_ELEM-1 && ci > 0) {
                    // First cell in element
                    cell_mask = MULTI_CELL_MASK << (cj-1)*BITS_PER_CELL;
                    cell_count_val =
                        ((w->data[ci] & cell_mask) >> (cj-2)*BITS_PER_CELL) |
                        (w->data[ci-1] & 0x3);
                } else if (cj == 0) {
                    // Last cell in element
                    cell_mask = MULTI_CELL_MASK >> BITS_PER_CELL;
                    cell_count_val = w->data[ci] & cell_mask;
                    if (ci < w->data_size-1) {
                        int high_bits = (CELLS_PER_ELEM-1)*BITS_PER_CELL;
                        cell_count_val =
                            cell_count_val |
                            ((w->data[ci+1] & (0x3 << high_bits)) >> high_bits);
                    }
                } else {
                    cell_mask = MULTI_CELL_MASK << (cj-1)*BITS_PER_CELL;
                    cell_count_val = (w->data[ci] & cell_mask) >> (cj-1)*BITS_PER_CELL;
                    if (x == 0) {
                        cell_count_val &= 0xf;
                    } else if (x == w->xlim-1) {
                        cell_count_val &= 0x3c;
                    }
                }

                cell_count += BIT_COUNTS[cell_count_val];
            }

            /* printf(" %2x %d ", cell_count_val, cell_count); */
            /* printf(" %x", w->data[i]); */
            /* putchar('\n'); */
            printf("%d ", cell_count);

            x++;
            if (x >= w->xlim) {
                putchar('\n');
                x = 0;
                y++;
                if (y >= w->ylim) {
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
