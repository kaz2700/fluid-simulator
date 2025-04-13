#include <SDL2/SDL.h>
#include "artist.h"
#include <stdio.h>
#include "math_functions.h"

int SCREEN_WIDTH = 600;
int SCREEN_HEIGHT = 600;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

float box_length = 1; //meters
float particle_radius;
float radius_multiplier;
SDL_Texture* particleTexture;
SDL_Texture* haloTexture;

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
    radius_multiplier = SCREEN_WIDTH / box_length * 35;
    particle_radius = 0.01;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    particleTexture = createParticleTexture();
    haloTexture = createHaloTexture();

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
        //drawParticle(particles[i]);
        drawHalo(particles[i]);
    }

    SDL_RenderPresent(renderer);
}

SDL_Texture* createParticleTexture() {
    SDL_Texture* circleTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 2 * particle_radius, 2 * particle_radius);
    SDL_SetTextureBlendMode(circleTex, SDL_BLENDMODE_BLEND);     //this enables blending in order for alpha to work when drawing

    SDL_SetRenderTarget(renderer, circleTex);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    // Draw white filled circle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = -particle_radius; i <= particle_radius; i++) {
        float h = sqrt(particle_radius * particle_radius - i * i);
        for (int j = -h; j <= h; j++) {
            SDL_RenderDrawPoint(renderer, particle_radius + i, particle_radius + j);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);

    return circleTex;
}

SDL_Texture* createHaloTexture() {
    int radius = radius_multiplier * particle_radius;
    SDL_Texture* circleTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 2 * radius, 2 * radius);
    SDL_SetTextureBlendMode(circleTex, SDL_BLENDMODE_BLEND);     //this enables blending in order for alpha to work when drawing

    SDL_SetRenderTarget(renderer, circleTex);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    // Draw white filled circle
    float alpha;
    float r;
    for (int i = -radius; i <= radius; i++) {
        float h = sqrt(radius * radius - i * i);
        for (int j = -h; j <= h; j++) {
            //TODO normalize brightness over area
            r = sqrt(i*i + j*j);
            alpha = 20 * (1 - pow(r / radius, 3));
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
            SDL_RenderDrawPoint(renderer, radius + i, radius + j);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);

    return circleTex;
}

void drawParticle(Particle* particle) {
    float* translated_position = translate_position(particle -> position);

    SDL_Rect dst = {
    .x = translated_position[0] - particle_radius,
    .y = translated_position[1] - particle_radius,
    .w = 2 * particle_radius,
    .h = 2 * particle_radius
    };

    int r = (int) 150 * vector_norm(particle -> velocity);

    SDL_SetTextureColorMod(particleTexture, r, 255 - r/2, 255 - r);  // for tinting
    SDL_RenderCopy(renderer, particleTexture, NULL, &dst);
}

void drawHalo(Particle* particle) {
    float* translated_position = translate_position(particle -> position);

    SDL_Rect dst = {
    .x = translated_position[0] - radius_multiplier * particle_radius,
    .y = translated_position[1] - radius_multiplier * particle_radius,
    .w = 2 * radius_multiplier * particle_radius,
    .h = 2 * radius_multiplier * particle_radius
    };

    SDL_SetTextureColorMod(haloTexture, 255, 255, 255);  // for tinting
    SDL_RenderCopy(renderer, haloTexture, NULL, &dst);
}