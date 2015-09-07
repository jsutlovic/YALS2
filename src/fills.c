#include "fills.h"

static void _fill0(world_cell_pos *wcp) {
    *(wcp->cell_val) = 0x2;
}

void fill0(world *w) {
    iter_world(w, _fill0);
}

static void _fill1(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x & 1) << 1;
}

void fill1(world *w) {
    iter_world(w, _fill1);
}

static void _fill2(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y) & 1) << 1;
}

void fill2(world *w) {
    iter_world(w, _fill2);
}

static void _fill3(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->x + wcp->y) & 3;
}

void fill3(world *w) {
    iter_world(w, _fill3);
}

static void _fill4(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x+1) & 1) << 1;
}

void fill4(world *w) {
    iter_world(w, _fill4);
}

static void _fill5(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->x + wcp->y + 1) & 1) << 1;
}

void fill5(world *w) {
    iter_world(w, _fill5);
}

static void _fill6(world_cell_pos *wcp) {
    *(wcp->cell_val) = (wcp->y & 1) << 1;
}

void fill6(world *w) {
    iter_world(w, _fill6);
}

static void _fill7(world_cell_pos *wcp) {
    *(wcp->cell_val) = ((wcp->y+1) & 1) << 1;
}

void fill7(world *w) {
    iter_world(w, _fill7);
}
