#include "math_functions.h"
#include <stdio.h>
#include <math.h>
#include "particle.h"
#include "string.h"


float distance_on_motion(Particle* particle1, Particle* particle2, float dt) {
    float* pos1 = particle1 -> position;
    float* pos2 = particle2 -> position;
    float* vel1 = particle1 -> velocity;
    float* vel2 = particle2 -> velocity;

    return sqrt( pow(dt * (vel2[0] - vel1[0]) + pos2[0]-pos1[0], 2) + pow(dt * (vel2[1] - vel1[1]) + pos2[1]-pos1[1], 2));
}

float radial_velocity(Particle* particle1, Particle* particle2){
    float* pos1 = particle1 -> position;
    float* pos2 = particle2 -> position;
    float* vel1 = particle1 -> velocity;
    float* vel2 = particle2 -> velocity;

    return 1 / distance(particle1, particle2) * ((pos2[0] - pos1[0]) * (vel2[0] - vel1[0]) + (pos2[1] - pos1[1]) * (vel2[1] - vel1[1]));
}

float* pointing_vector(Particle* particle1, Particle* particle2) {
    float x1 = particle1 -> position[0];
    float y1 = particle1 -> position[1];
    float x2 = particle2 -> position[0];
    float y2 = particle2 -> position[1];

    static float pointing_vector[2];

    pointing_vector[0] = x2-x1;
    pointing_vector[1] = y2-y1;

    return pointing_vector;
}

float* normalized_vector(float* vector) {

    static float normalized_vector[2];

    normalized_vector[0] = vector[0]/vector_norm(vector);
    normalized_vector[1] = vector[1]/vector_norm(vector);

    return normalized_vector;

}

float vector_norm(float* vector) {
    return sqrt(pow(vector[0], 2) + pow(vector[1], 2));
}

float distance(Particle* particle1, Particle* particle2) {
    float x1 = particle1 -> position[0];
    float y1 = particle1 -> position[1];
    float x2 = particle2 -> position[0];
    float y2 = particle2 -> position[1];

    return sqrt( pow(x2-x1, 2) + pow(y2-y1, 2));
}