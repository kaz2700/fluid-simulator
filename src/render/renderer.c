#include <SDL2/SDL.h>
#include "render/renderer.h"
#include "spatial/grid.h"
#include "core/math_utils.h"
#include "core/linked_list.h"
#include "core/profiler.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int window_width = 600;
int window_height = 600;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

float domain_size = 1.0f;
static float particle_visual_radius = 0.005f;
static float pixels_per_meter;
static SDL_Texture* particle_texture = NULL;

static SDL_Texture* create_particle_texture(void);
static void draw_particle(Particle* p);

int init_renderer(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("Fluid Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              window_width, window_height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    particle_visual_radius = 0.005f;
    pixels_per_meter = window_width / domain_size;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    particle_texture = create_particle_texture();

    return 1;
}

void shutdown_renderer(void) {
    SDL_DestroyTexture(particle_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = NULL;
    window = NULL;
    SDL_Quit();
}

void render_frame(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Node* current_partition = get_all_partitions();
    while (current_partition != NULL) {
        Node* current = current_partition->item;
        while (current != NULL) {
            draw_particle((Particle*)current->item);
            current = current->next;
        }
        current_partition = current_partition->next;
    }

    SDL_RenderPresent(renderer);
}

static SDL_Texture* create_particle_texture(void) {
    int radius = (int)(particle_visual_radius * pixels_per_meter);
    SDL_Texture* circle_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_TARGET, 2 * radius, 2 * radius);
    SDL_SetTextureBlendMode(circle_tex, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, circle_tex);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    for (int i = -radius; i <= radius; i++) {
        float h = sqrt(radius * radius - i * i);
        for (int j = (int)-h; j <= (int)h; j++) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, radius + i, radius + j);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    return circle_tex;
}

static void draw_particle(Particle* p) {
    float x = (domain_size - p->position[0]) * window_width;
    float y = (domain_size - p->position[1]) * window_height;
    int radius = (int)(particle_visual_radius * pixels_per_meter);

    SDL_Rect dst = {
        .x = (int)x - radius,
        .y = (int)y - radius,
        .w = 2 * radius,
        .h = 2 * radius
    };

    float speed = vector_norm(p->velocity);
    int r = (int)(150.0f * speed);
    int g = 255 - r / 2;
    int b = 255 - r;

    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (b < 0) b = 0;

    SDL_SetTextureColorMod(particle_texture, r, g, b);
    SDL_RenderCopy(renderer, particle_texture, NULL, &dst);
}

void render_frame_with_profiler(Profiler* prof, int particle_count) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Node* current_partition = get_all_partitions();
    while (current_partition != NULL) {
        Node* current = current_partition->item;
        while (current != NULL) {
            draw_particle((Particle*)current->item);
            current = current->next;
        }
        current_partition = current_partition->next;
    }

    // Draw profiler metrics overlay
    profiler_draw_metrics(renderer, prof, particle_count);

    SDL_RenderPresent(renderer);
}
