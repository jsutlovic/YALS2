#include "fills.h"

static void _fill_empty(world_cell_pos *wcp) {
    *(wcp->cell_val) = 0x0;
}

static void _fill_even_in_row(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x & 1) << 1;
}

static void _fill_even_in_world(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y) & 1) << 1;
}

static void _fill_mod4(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x + wcp->y) & 2;
}

static void _fill_odd_in_row(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x+1) & 1) << 1;
}

static void _fill_odd_in_world(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y + 1) & 1) << 1;
}

static void _fill_even_row(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->y & 1) << 1;
}

static void _fill_odd_row(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->y+1) & 1) << 1;
}

static void _fill_test_cell(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->y < 1 && wcp->x < 1) << 1;
}

static void _fill_full(world_cell_pos *wcp) {
    *(wcp->cell_val) = 0x2;
}

static void _fill_rand(world_cell_pos *wcp) {
    *(wcp->cell_val) = rand() & 2;
}

void fill(world *w, fill_type fill) {
    iter_world_func_type f_it;

    switch(fill) {
        case(EMPTY): f_it = _fill_empty; break;
        case(EVEN_IN_ROW): f_it = _fill_even_in_row; break;
        case(EVEN_IN_WORLD): f_it = _fill_even_in_world; break;
        case(MOD_4): f_it = _fill_mod4; break;
        case(ODD_IN_ROW): f_it = _fill_odd_in_row; break;
        case(ODD_IN_WORLD): f_it = _fill_odd_in_world; break;
        case(EVEN_ROW): f_it = _fill_even_row; break;
        case(ODD_ROW): f_it = _fill_odd_row; break;
        case(TEST_CELL): f_it = _fill_test_cell; break;
        case(FULL): f_it = _fill_full; break;
        case(RANDOM): f_it = _fill_rand; break;
        default: return;
    }

    w->generation = 0;
    w->state = CALC;
    iter_world(w, f_it);
}
