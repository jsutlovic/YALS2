#include <stdio.h>
#include "game.h"
#include "rules.h"
#include "fills.h"

int main() {
    game *g = init_game(10, 10, conways_life);
    printf("World size: %lu\n", g->w->data_size);
    fill1(g->w);
    print_world(g->w);
    game_half_step(g);
    destroy_game(g);
    return EXIT_SUCCESS;
}
