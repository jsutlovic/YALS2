#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "world.h"
#include "game.h"
#include "fills.h"


static unsigned long int parse_int_opt(char *optval) {
    long int val = strtol(optval, NULL, 10);
    if (val <= 0) {
        fprintf(stderr, "Invalid numeric value: %s\n", optval);
        exit(EXIT_FAILURE);
    }

    return val;
}

int main(int argc, char **argv) {
    int c;
    int pflag = 0, tflag = 0;
    unsigned long int xlim = 160, ylim = 100, ilim = 1, fill_type = 3;
    char *fopt = NULL;

    const char *optstr = "tn:w:x:h:y:f:pi:";

    while ( (c = getopt(argc, argv, optstr)) != -1 ) {
        switch (c) {
            case 'p':
                // Profiling flag
                pflag = 1;
                break;
            case 'i':
                // Iteration count
                ilim = parse_int_opt(optarg);
                break;
            case 't':
                // Text flag
                tflag = 1;
                break;
            case 'n':
                // Fill type
                fill_type = parse_int_opt(optarg);
                break;
            case 'w':
            case 'x':
                // World width
                xlim = parse_int_opt(optarg);
                break;
            case 'h':
            case 'y':
                // World height
                ylim = parse_int_opt(optarg);
                break;
            case 'f':
                // File option (read/write)
                fopt = optarg;
                break;
            case '?':
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }
    if (optind < argc) {
        printf("Ignoring non-option args: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        putchar('\n');
    }

    // Declare world
    world *w = NULL;

    if (pflag) {
        if (ilim <= 0) {
            fprintf(stderr, "Invalid iteration count: %lu\n", ilim);
            exit(EXIT_FAILURE);
        }
        puts("Profiling");

        unsigned long iterations = ilim * 1000;
        printf("Iterations: %lu\n", iterations);

        w = init_world(200, 200);

        printf("World size: %lu\n", w->data_size);
        fill(w, fill_type);

        puts("Start!");
        for (unsigned long i = 0; i < iterations; i++) {
            world_step(w);
        }
        puts("End!");
    } else {
        if (fopt != NULL) {
            printf("Opening and saving to file %s\n", fopt);
            // Attempt to read world and set xlim/ylim
            w = read_from_file(fopt);

            if (w != NULL) {
                xlim = w->xlim;
                ylim = w->ylim;
            }
        }

        if (xlim == 0 || ylim == 0) {
            fputs("Need a positive width and height\n", stderr);
            exit(EXIT_FAILURE);
        } else if (w == NULL) {
            // Create an empty world
            w = init_world(xlim, ylim);
            fill(w, fill_type);
        }

        // Definitely should have a world at this point
        printf("World size: %lu\n", w->data_size);

        if (tflag) {
            print_world(w);
            for (int i = 0; i < 5; i++) {
                world_half_step(w);
                print_world(w);
                world_half_step(w);
                print_world(w);
            }
        } else {
            game *g = init_game_from_world(w);
            setup_game(g, 1280, 720);
            start_game(g);

            if (fopt != NULL) {
                write_to_file(fopt, g->w);
            }

            destroy_game(g);
        }

    }

    destroy_world(w);

    return EXIT_SUCCESS;
}

