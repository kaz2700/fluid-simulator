#include "particle.h"
#include <SDL2/SDL.h>


#ifndef artist_header
#define artist_header

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern float box_length; //meters

int init();
void close_sdl();
void drawParticle(Particle* particle);
void drawHalo(float* position, float radius);
float* translate_position(float* position);
void draw(Particle** particles, int num_of_particles);


#endif