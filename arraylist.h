#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <threads.h>
#include "particle.h"

#ifndef arraylist_header
#define arraylist_header

typedef struct Node {
    void* item;
    struct Node* next;
} Node;

Particle* getItemFromIndex(int index);
void addToLinkedList(Node** listHead, void* item);
void createParticleList(int num_of_particles);
void removeFromLinkedList(Node** listHead, Node* removingItem);
void createSpacePartitions(int num_of_partitions);
Node* getParticleHeadNode();
Node* getSpacePartitionHeadNode();
extern int spacePartitions;
#endif