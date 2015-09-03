#include <stdio.h>
#include "world.h"
#include "game.h"
#include "fills.h"

#define PROFILE 0
#define GAME 1

int main() {
#if GAME

    game *g = init_game(10, 10);
    fill1(g->w);
    setup_game(g, 640, 480);

    start_game(g);

    destroy_game(g);

#else // GAME OFF

#if PROFILE
    world *w = init_world(200, 200);
#else
    world *w = init_world(10, 10);
#endif

    printf("World size: %lu\n", w->data_size);
    fill1(w);

#if PROFILE
    puts("Start!");
    for (int i = 0; i < 1000; i++) {
        world_step(w);
    }
    puts("End!");
#else // PROFILE OFF
    print_world(w);
    for (int i = 0; i < 5; i++) {
        world_half_step(w);
        print_world(w);
        world_half_step(w);
        print_world(w);
    }
#endif

    destroy_world(w);

#endif
    return EXIT_SUCCESS;
}
