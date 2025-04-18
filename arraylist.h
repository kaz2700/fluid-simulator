#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <threads.h>
#include "particle.h"

#ifndef arraylist_header
#define arraylist_header

typedef struct Node {
    Particle* particle;
    struct Node* next;
} Node;

Particle* getParticleFromIndex(int index);
void addToLinkedList(Particle* particle);
void createParticleList(int num_of_particles);
Node* getHeadNode();

#endif