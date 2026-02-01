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
#include "core/profiler.h"

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

    Profiler profiler;
    profiler_init(&profiler);

    int should_quit = 0;
    SDL_Event event;

    while (!should_quit) {
        while (SDL_PollEvent(&event) != 0)
            if (event.type == SDL_QUIT)
                should_quit = 1;

        profiler_start_frame(&profiler);

        profiler_start_physics(&profiler);
        physics_step(time_step);
        profiler_end_physics(&profiler);

        profiler_start_render(&profiler);
        render_frame_with_profiler(&profiler, 10000);
        profiler_end_render(&profiler);

        profiler_end_frame(&profiler);

        usleep((useconds_t)(1000000 * time_step));
    }

    cleanup_grid();
    shutdown_renderer();

    return 0;
}
