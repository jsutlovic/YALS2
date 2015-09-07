#include <limits.h>
#include <stdio.h>
#include "world.h"

#define DEBUG 0
#define DEBUG2 0

static const char DISPLAY_CHARS[4] = { ' ', 'o', '*', 'O' };
/*
 * Number of set bits in the lowest 3 'even' bit positions.
 * Using a number masked by 0x2a as an index to this array,
 * the value of the array at that index is the number of bits set
 * in the number used to index.
 * e.g. BIT_COUNTS[0x2 & 0x2a] == 1, BIT_COUNTS[0x537 & 0x2a] == 2
 */
static const char BIT_COUNTS[BIT_COUNT_LEN] = {
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

    w->data = calloc(w->data_size + 1, sizeof(world_store));
    return w;
}

void destroy_world(world *w) {
    free(w->data);
    free(w);
}

void iter_world(world *w, iter_world_func_type itf) {
    size_t x = 0, y = 0;
    world_store cell_val, cell_mask;
    world_cell_pos wcp;
    wcp.w = w;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = CELLS_PER_ELEM-1; j >= 0; j--) {
            cell_mask = SINGLE_CELL_MASK << j*BITS_PER_CELL;
            cell_val = (w->data[i] & cell_mask) >> j*BITS_PER_CELL;

            wcp.x = x;
            wcp.y = y;
            wcp.cell_val = &cell_val;
            itf(&wcp);
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

static void _print_world_it(world_cell_pos *wcp) {
    size_t index = wcp->w->state ? *(wcp->cell_val) : (*(wcp->cell_val) & 2) | (*(wcp->cell_val) >> 1);
    putchar(DISPLAY_CHARS[index]);
    if (wcp->x == wcp->w->xlim-1) {
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

static void _shift_next_state(world *w) {
    for (size_t i = 0; i < w->data_size; i++) {
        w->data[i] = (w->data[i] << 1) & CURR_CELL_MASK;
    }

    w->generation++;
    w->state = CALC;
}

static void _calc_next_state(world *w) {
    size_t x = 0, y = 0;
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
#if DEBUG
                    putchar('1');
                    printf(" m: %08x ", cell_mask);
#endif
                    // Get first 2 cells in current element
                    // Add last cell of previous element
                    cell_count_val =
                        ((w->data[ci] >> (cj-1)*BITS_PER_CELL) |
                         (w->data[ci-1] << 2*BITS_PER_CELL)) &
                        MULTI_CELL_MASK;
                } else if (cj == 0) {
                    // Last cell in element
#if DEBUG
                    putchar('2');
                    printf(" m: %08x ", cell_mask);
#endif
                    // Get last 2 cells in current element
                    // Get first cell of next element
                    cell_count_val =
                        ((w->data[ci] << BITS_PER_CELL) |
                        (w->data[ci+1] >> (CELLS_PER_ELEM-1)*BITS_PER_CELL)) &
                        MULTI_CELL_MASK;
                } else {
                    // Get the surrounding 2 cells
                    cell_count_val =
                        (w->data[ci] >> (cj-1)*BITS_PER_CELL) &
                        MULTI_CELL_MASK;
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

            cell_mask = NEXT_STATE_MASK << j*BITS_PER_CELL;
            switch(cell_count) {
                case 3: cell_val = 1 << j*BITS_PER_CELL; break;
                case 4: cell_val = (w->data[i] >> 1) & cell_mask; break;
                default: cell_val = 0; break;
            }
            w->data[i] = (w->data[i] & (~cell_mask)) | cell_val;
#if 0
            printf("%d:%08x:%x\n", cell_count, cell_mask, cell_val);
#endif

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

    w->state = SHIFT;
}

void world_half_step(world *w) {
    switch (w->state) {
        case CALC:  _calc_next_state(w); break;
        case SHIFT: _shift_next_state(w); break;
    }
}

void world_step(world *w) {
    if (w->state == CALC) {
        world_half_step(w);
    }
    world_half_step(w);
}
