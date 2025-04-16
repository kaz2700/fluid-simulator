#include <stddef.h>
#include <stdio.h>
#include "particle.h"
#include "artist.h"
#include "arraylist.h"

Node* head = NULL;

Particle* getParticleFromIndex(int index) {
    int i = 0;
    Node* current = head;

    while (current != NULL) {
        if (index == i)
            return current->particle;

        current = current->next;
        i++;
    }

    return NULL;
}

void addToLinkedList(Particle* particle) {
    Node* newNode = malloc(sizeof(Node));
    newNode->particle = particle; 
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
        return;
    }

    Node* current = head;

    while (current->next != NULL)
        current = current->next;

    current->next = newNode;
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

        addToLinkedList(particle);
    }
}

Node* getHeadNode() {
    return head;
}