#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>


#ifndef arraylist_header
#define arraylist_header

#define NUM_PARTICLES 100

void* particles[NUM_PARTICLES] = { NULL };

void init_arraylist(void** particles, int num_of_particles);

#endif