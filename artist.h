#include "particle.h"
#include <SDL2/SDL.h>


#ifndef artist_header
#define artist_header

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern float box_length; //meters
extern float particle_radius;
extern SDL_Texture* particleTexture;
extern SDL_Texture* haloTexture;
extern float radius_multiplier;

int init();
void close_sdl();
void drawParticle(Particle* particle);
void drawHalo(Particle* particle);
float* translate_position(float* position);
void draw(Particle* particles[], int num_of_particles);
SDL_Texture* createParticleTexture();
SDL_Texture* createHaloTexture();


#endif