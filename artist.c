#include <SDL2/SDL.h>
#include "artist.h"
#include <stdio.h>
#include "math_functions.h"

int SCREEN_WIDTH = 600;
int SCREEN_HEIGHT = 600;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

float box_length = 1; //meters

int init() {
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return 0;
    }

    window = SDL_CreateWindow( "Fluid Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    
    if ( window == NULL ) {
        printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
    return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    
    if ( renderer == NULL ) {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }

    //this enables blending in order for alpha to work when drawing
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return 1;
}

void close_sdl() {
	SDL_DestroyWindow( window );
	window = NULL;

	SDL_Quit();
}

float* translate_position(float* position) {
    float x = (1 - (box_length - position[0])) * SCREEN_WIDTH;
    float y = (box_length - position[1]) * SCREEN_HEIGHT;

    static float translated_position[2];

    translated_position[0] = x;
    translated_position[1] = y;

    return translated_position;
}

void draw(Particle* particles[], int num_of_particles){
    //clear screen
    SDL_SetRenderDrawColor(renderer,0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for(int i = 0; i < num_of_particles; i++) {
        drawParticle(particles[i]);
        //drawHalo(particles[i].position, particles[i].radius*100);
    }

    SDL_RenderPresent(renderer);
}

void drawParticle(Particle* particle) {
    float radius = particle -> radius * SCREEN_WIDTH/box_length;
    float* translated_position = translate_position(particle -> position);
    float h;
    int r;
    for(int i = -radius; i <= radius; i++) {
        h = sqrt( pow(radius, 2) - pow(i, 2));
        for(int j = -h; j <= h; j++) {
            r = (int) 100 * sqrt(pow(particle -> velocity[0], 2) + pow(particle -> velocity[1], 2));
            SDL_SetRenderDrawColor(renderer, r, 255-r, 255-r, 255);
            SDL_RenderDrawPoint(renderer, translated_position[0] + i, translated_position[1] + j);
        }
    }
}

void drawHalo(float* position, float radius) {
    float* translated_position = translate_position(position);
    float h;
    float alpha;
    int range = radius * 10;
    float r;
    for(int i = -range; i <= range; i++) {
        h = sqrt( pow(range, 2) - pow(i, 2));
        for(int j = -h; j <= h; j++) {
            r = sqrt(pow(i, 2) + pow(j, 2));
            if (r == 0)
                alpha = 1;
            else
                alpha = 1/(r);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha * 255);
            SDL_RenderDrawPoint(renderer, translated_position[0] + i, translated_position[1] + j);
        }
    }
}