#include <SDL2/SDL.h>
#include <stdio.h>
#include "physics.h"
#include <unistd.h>
#include "artist.h"
#include "arraylist.h"
#include "space_partition.h"

float dt = 0.1; //seconds

int main( int argc, char* args[] ) {
	if ( !init() ) {
		printf( "Failed to initialize!\n" );
        return 0;
	}
    createSpacePartitions(4); //this first
    //createParticleList(1);

    int l = getListLength(getSpacePartitionList());
    printf("SpacePartitionListLength: %d\n", l);

    int quit = 0;
    SDL_Event e;
    while( !quit ) {
        while( SDL_PollEvent( &e ) != 0 )
            if( e.type == SDL_QUIT )
                quit = 1;

        tick(dt);
        draw();

        usleep(1000000 * dt); //microseconds
    }

	close_sdl();

	return 0;
}