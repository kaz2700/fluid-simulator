#define _GNU_SOURCE
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "physics/integrator.h"
#include "render/renderer.h"
#include "spatial/grid.h"
#include "spatial/particle_factory.h"
#include "core/linked_list.h"

static const float time_step = 0.01f;

int main(void) {
    if (!init_renderer()) {
        fprintf(stderr, "Failed to initialize renderer!\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    init_grid(256);
    create_particles(10000);

    int partition_count = list_count(get_all_partitions());
    printf("SpacePartitionListLength: %d\n", partition_count);

    int should_quit = 0;
    SDL_Event event;

    while (!should_quit) {
        while (SDL_PollEvent(&event) != 0)
            if (event.type == SDL_QUIT)
                should_quit = 1;

        physics_step(time_step);
        render_frame();
        usleep((useconds_t)(1000000 * time_step));
    }

    cleanup_grid();
    shutdown_renderer();

    return 0;
}
