#include <stdio.h>
#include "game.h"
#include "rules.h"
#include "fills.h"

#define PROFILE 0

int main() {
#if !PROFILE
    game *g = init_game(10, 10, conways_life);
#else
    game *g = init_game(200, 200, conways_life);
#endif
    printf("World size: %lu\n", g->w->data_size);
    fill1(g->w);
#if !PROFILE
    print_world(g->w);
    for (int i = 0; i < 5; i++) {
        game_half_step(g);
        print_world(g->w);
        game_half_step(g);
        print_world(g->w);
    }
#else
    puts("Start!");
    for (int i = 0; i < 1000; i++) {
        game_step(g);
    }
    puts("End!");
#endif
    destroy_game(g);
    return EXIT_SUCCESS;
}
