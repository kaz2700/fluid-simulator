#ifndef PARTICLE_H
#define PARTICLE_H

typedef struct Particle {
    float position[2];
    float velocity[2];
    float acceleration[2];
    float radius;
    float mass;
    float charge;
} Particle;

#endif
