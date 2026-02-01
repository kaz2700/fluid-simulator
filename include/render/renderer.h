#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "core/particle.h"

// Forward declaration to avoid circular include
struct Profiler;

extern int window_width;
extern int window_height;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern float domain_size;

int init_renderer(void);
void shutdown_renderer(void);
void render_frame(void);
void render_frame_with_profiler(struct Profiler* prof, int particle_count);

#endif
