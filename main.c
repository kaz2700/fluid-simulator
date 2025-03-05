#include <SDL2/SDL.h>
#include <stdio.h>
#include "physics.h"
#include "particle.h"
#include <unistd.h>
#include "artist.h"
#include <time.h>
#include "arraylist.h"

float dt = 0.01; //seconds

int main( int argc, char* args[] ) {
	if ( !init() ) {
		printf( "Failed to initialize!\n" );
        return 0;
	}

    init_arraylist(particles, NUM_PARTICLES);

    Particle** part = (Particle**) particles;

    int quit = 0;
    SDL_Event e;
    while( !quit ) {
        while( SDL_PollEvent( &e ) != 0 )
            if( e.type == SDL_QUIT )
                quit = 1;

        tick(part, dt, NUM_PARTICLES);
        draw(part, NUM_PARTICLES);
        
        usleep(1000000 * dt); //microseconds
    }

	close_sdl();

	return 0;
}