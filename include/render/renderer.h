#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "core/particle.h"

extern int window_width;
extern int window_height;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern float domain_size;

int init_renderer(void);
void shutdown_renderer(void);
void render_frame(void);

#endif
