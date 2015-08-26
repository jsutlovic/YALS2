#include <stdio.h>
#include "game.h"
#include "rules.h"
#include "fills.h"

int main() {
    game *g = init_game(160, 30, conways_life);
    printf("World size: %lu\n", g->w->data_size);
    fill2(g->w);
    print_world(g->w);
    destroy_game(g);
    return EXIT_SUCCESS;
}
