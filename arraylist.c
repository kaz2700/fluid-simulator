#include <stddef.h>
#include <stdio.h>
#include "particle.h"
#include "artist.h"

void init_arraylist(void** particles, int num_of_particles) {
    for(int i = 0; i < num_of_particles; i++) {
        particles[i] = malloc(sizeof(Particle));
        if (!particles[i]) {
            perror("Memory allocation failed");
            exit(1);
        }
    }

    Particle original_particle;

    float x_init =  box_length/4;
    float y_init = box_length/4;

    original_particle.radius = 0.01;
    original_particle.mass = 1;
    original_particle.charge = 0.05;
    original_particle.id = 0;

    int i_max = 3;

    float x = x_init;
    float y = y_init;
    for(int i = 0; i < num_of_particles; i++) {
        Particle* particle = (Particle*) particles[i];
        *particle = original_particle;

        x = x_init + 2 * particle -> radius * (i % i_max);
        y = y_init + 2 * particle -> radius * (i / i_max);

        if (i % i_max == 0) {
            x = x_init;
        }
                printf("x %f\n", (x - x_init) / (2 * particle -> radius));
                printf("y %f\n\n", (y - y_init) / (2 * particle -> radius));

                particle -> position[0] = x;
                particle -> position[1] = y;
                particle -> velocity[0] = 0;//(float) random() / RAND_MAX;
                particle -> velocity[1] = 0;//(float) random() / RAND_MAX;
                particle -> id = i;
        }
}