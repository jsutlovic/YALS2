#include <stdio.h>
#include <stdlib.h>
#include "world.h"

int main() {
    world *w = init_world(10, 10);
    printf("World size: %lu\n", w->data_size);
    destroy_world(w);
    return EXIT_SUCCESS;
}
