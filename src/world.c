#include "world.h"

static const uint16_t MAGIC = 0xf0de;

static const char DISPLAY_CHARS[4] = { '.', 'o', '*', 'O' };
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

    w->cell_count = xlim * ylim;
    w->data_size = ( w->cell_count * (float) BITS_PER_CELL ) / ( sizeof(world_store) * CHAR_BIT ) + .969;

    w->data = calloc(w->data_size + 1, sizeof(world_store));
    w->temp_calc = calloc(w->data_size + 1, sizeof(world_store));
    return w;
}

void destroy_world(world *w) {
    free(w->data);
    free(w->temp_calc);
    free(w);
}

void invert_cell(world_cell_pos *p) {
    size_t i;
    int j;
    size_t cell_idx = (p->y * p->w->xlim) + p->x;

    i = cell_idx >> IDX_DIV;
    j = cell_idx & OFFSET_MASK;

    world_store cell_mask = (world_store) SINGLE_CELL_MASK << j*BITS_PER_CELL;
    world_store cell_val = (p->w->data[i] >> j*BITS_PER_CELL) & SINGLE_CELL_MASK;
    p->w->data[i] = (p->w->data[i] & ~cell_mask) | ((~cell_val << j*BITS_PER_CELL) & cell_mask);
}

void iter_world(world *w, iter_world_func_type itf) {
    size_t x = 0, y = 0;
    world_store cell_val, cell_mask;
    world_cell_pos wcp;
    wcp.w = w;

    for (size_t i = 0; i < w->data_size; i++) {
        for (int j = 0; j < CELLS_PER_ELEM; j++) {
            cell_mask = (world_store) SINGLE_CELL_MASK << j*BITS_PER_CELL;
            cell_val = (w->data[i] >> j*BITS_PER_CELL) & SINGLE_CELL_MASK;

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
    printf("World %ux%u, state: %s, gen %u:\n",
            w->xlim,
            w->ylim,
            w->state ? "SHIFT" : "CALC",
            w->generation);
    iter_world(w, _print_world_it);
}


/*
 * World into byte stream
 *   - begin stream magic number
 *   - xlim, ylim
 *   - generation
 *   - state
 *   - world_data
 */
char *serialize_world(world *w, size_t *len) {
    size_t out_size =
        sizeof(MAGIC) +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        sizeof(uint16_t) +
        (w->data_size * sizeof(world_store));

    size_t offset = 0;
    char *s_w = calloc(out_size, sizeof(char));

    _ser_uint16(s_w, offset, MAGIC);
    offset += sizeof(MAGIC);

    _ser_uint32(s_w, offset, w->xlim);
    offset += sizeof(uint32_t);

    _ser_uint32(s_w, offset, w->ylim);
    offset += sizeof(uint32_t);

    _ser_uint32(s_w, offset, w->generation);
    offset += sizeof(uint32_t);

    _ser_uint16(s_w, offset, (uint16_t) w->state);
    offset += sizeof(uint16_t);

    for (size_t i = 0; i < w->data_size; ++i) {
        _ser_uint32(s_w, offset, w->data[i]);
        offset += sizeof(uint32_t);
    }

    *len = out_size;

    return s_w;
}

static void _shift_next_state(world *w) {
    for (size_t i = 0; i < w->data_size; i++) {
        w->data[i] = (w->data[i] << 1) & CURR_CELL_MASK;
    }

    w->generation++;
    w->state = CALC;
}

static void _calc_next_state(world *w) {
    size_t x, c, cc, i, j, ci, cj;
    world_store cell_val, cell_mask, cell_count_val;
    unsigned char three_cells, row_cell_count;

    x = 0;
    i = 0;
    // This contains the current cell and the two cells surrounding it
    // We start off with the first cell
    three_cells = w->data[i] & SINGLE_CELL_MASK;

    for (cc = 0, c = 1; cc < w->cell_count; ++cc, ++c) {
        // The next cell's index and offset (for reading)
        i = c >> IDX_DIV;
        j = c & OFFSET_MASK;
        // Current cell's index and offset (for writing)
        ci = cc >> IDX_DIV;
        cj = cc & OFFSET_MASK;

        // Shift the next cell into our cell buffer
        three_cells = (three_cells << BITS_PER_CELL) |
            ((w->data[i] >> j*BITS_PER_CELL) & SINGLE_CELL_MASK);
        // We only care about the current 3 cells
        cell_count_val = three_cells & MULTI_CELL_MASK;

        // Don't use the left cell if the row has started
        // Don't use the right cell if the row has ended
        if (x == 0) {
            cell_count_val &= START_ROW_MASK;
        } else if (x == w->xlim-1) {
            cell_count_val &= END_ROW_MASK;
        }

        // Get the bit count of the three cells
        row_cell_count = BIT_COUNTS[cell_count_val];
        cell_mask = (world_store) SINGLE_CELL_MASK << cj*BITS_PER_CELL;

        // Set the surrounding cell count to our scratch area
        w->temp_calc[ci] = (w->temp_calc[ci] & ~cell_mask) |
            ((world_store) row_cell_count << cj*BITS_PER_CELL);

        ++x;
        if (x >= w->xlim) {
            x = 0;
        }
    }

    char sum9;
    for (c = 0; c < w->cell_count; ++c) {
        sum9 = 0;

        // Previous row three-count
        if (c >= w->xlim) {
            cc = c - w->xlim;
            ci = cc >> IDX_DIV;
            cj = (cc & OFFSET_MASK) * BITS_PER_CELL;
            sum9 += (w->temp_calc[ci] >> cj) & SINGLE_CELL_MASK;
        }

        // Next row three-count
        cc = c + w->xlim;
        if (cc < w->cell_count) {
            ci = cc >> IDX_DIV;
            cj = (cc & OFFSET_MASK) * BITS_PER_CELL;
            sum9 += (w->temp_calc[ci] >> cj) & SINGLE_CELL_MASK;
        }

        // Current row three-count
        ci = c >> IDX_DIV;
        cj = (c & OFFSET_MASK) * BITS_PER_CELL;
        sum9 += (w->temp_calc[ci] >> cj) & SINGLE_CELL_MASK;

        cell_mask = (world_store) NEXT_STATE_MASK << cj;
        // Set cell state based on the current cell and all surrounding
        // Conway's Life rules
        switch(sum9) {
            case 3: cell_val = (world_store) 1 << cj; break;
            case 4: cell_val = (w->data[ci] >> 1) & cell_mask; break;
            default: cell_val = 0; break;
        }
        w->data[ci] = (w->data[ci] & (~cell_mask)) | cell_val;
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
