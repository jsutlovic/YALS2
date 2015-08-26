#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "game.h"

int main() {
    world *w = init_world(100, 100);
    printf("World size: %lu\n", w->data_size);
    fill1(w);
    print_world(w);
    destroy_world(w);
    return EXIT_SUCCESS;
}
