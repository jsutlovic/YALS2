#include "fills.h"

static void _fill0(world_cell_pos *wcp) {
    *(wcp->cell_val) = 0x2;
}

static void _fill1(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x & 1) << 1;
}

static void _fill2(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y) & 1) << 1;
}

static void _fill3(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x + wcp->y) & 3;
}

static void _fill4(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x+1) & 1) << 1;
}

static void _fill5(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y + 1) & 1) << 1;
}

static void _fill6(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->y & 1) << 1;
}

static void _fill7(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->y+1) & 1) << 1;
}

void fill(world *w, fill_type fill) {
    iter_world_func_type f_it;

    switch(fill) {
        case(0): f_it = _fill0; break;
        case(1): f_it = _fill1; break;
        case(2): f_it = _fill2; break;
        case(3): f_it = _fill3; break;
        case(4): f_it = _fill4; break;
        case(5): f_it = _fill5; break;
        case(6): f_it = _fill6; break;
        case(7): f_it = _fill7; break;
        default: return;
    }

    iter_world(w, f_it);
}
