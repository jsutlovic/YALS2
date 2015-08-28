#include <stdio.h>
#include "game.h"
#include "rules.h"
#include "fills.h"

int main() {
    game *g = init_game(200, 200, conways_life);
    printf("World size: %lu\n", g->w->data_size);
    fill1(g->w);
    puts("Start!");
    for (int i = 0; i < 8000; i++) {
        game_step(g);
    }
    puts("End!");
    destroy_game(g);
    return EXIT_SUCCESS;
}
