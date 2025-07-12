#include <stddef.h>
#include <stdio.h>
#include "particle.h"
#include "artist.h"
#include "arraylist.h"
#include "space_partition.h"

Node* particleHead = NULL;
Node* spaceHead = NULL;
int spacePartitions = 0;

Particle* getItemFromIndex(int index) {
    int i = 0;
    Node* current = particleHead;

    while (current != NULL) {
        if (index == i)
            return current->item;

        current = current->next;
        i++;
    }

    return NULL;
}

void addToLinkedList(Node** listHead, void* item) {
    Node* newNode = malloc(sizeof(Node));
    newNode->item = item; 
    newNode->next = NULL;

    if (*listHead == NULL) {
        *listHead = newNode;
        printf("head\n");
        return;
    }

    Node* current = *listHead;

    while (current->next != NULL) //TODO can remove next?
        current = current->next;
    current->next = newNode;
} 

void removeFromLinkedList(Node** listHead, Node* removingItem) {
    if (*listHead == NULL) {
        printf("error: empty LinkedList\n");
        return;
    }

    Node* current = *listHead;

    while (current->next != NULL) {
        if (current->next == removingItem) {
            current->next = current->next->next;
            printf("removed\n");
            return;
        }
        current = current->next;
    }
} 

void createSpacePartitions(int num_of_partitions) {
    spacePartitions = num_of_partitions;
    for (int i = 0; i < num_of_partitions; i++) {
        addToLinkedList(&spaceHead, NULL);
    }
}

void createParticleList(int num_of_particles) {
    Particle original_particle;

    float x_init =  box_length/4;
    float y_init = box_length/4;

    original_particle.radius = 0.01;
    original_particle.mass = 10;
    original_particle.charge = 0.05;

    int i_max = 3;

    float x = x_init;
    float y = y_init;

    for(int i = 0; i < num_of_particles; i++) {
        Particle* particle = malloc(sizeof(Particle));
        *particle = original_particle;

        x = x_init + 2 * particle->radius * (i % i_max);
        y = y_init + 2 * particle->radius * (i / i_max);

        if (i % i_max == 0)
            x = x_init;

        particle->position[0] = x;
        particle->position[1] = y;
        particle->velocity[0] = (float) random() / RAND_MAX;
        particle->velocity[1] = (float) random() / RAND_MAX;

        assignSpacePartition(particle);
    }

}

Node* getParticleHeadNode() {
    return particleHead;
}

Node* getSpacePartitionHeadNode() {
    return spaceHead;
}