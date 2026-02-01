#include "core/math_utils.h"
#include <math.h>

float distance_on_motion(Particle* particle1, Particle* particle2, float dt) {
    float dx = particle2->position[0] - particle1->position[0];
    float dy = particle2->position[1] - particle1->position[1];
    float dvx = particle2->velocity[0] - particle1->velocity[0];
    float dvy = particle2->velocity[1] - particle1->velocity[1];

    return sqrt(pow(dvx * dt + dx, 2) + pow(dvy * dt + dy, 2));
}

void pointing_vector(Particle* particle1, Particle* particle2, float* result) {
    result[0] = particle2->position[0] - particle1->position[0];
    result[1] = particle2->position[1] - particle1->position[1];
}

void normalized_vector(const float* vector, float* result) {
    float norm = vector_norm(vector);
    if (norm > 0) {
        result[0] = vector[0] / norm;
        result[1] = vector[1] / norm;
    } else {
        result[0] = 0;
        result[1] = 0;
    }
}

float vector_norm(const float* vector) {
    return sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
}

float distance(Particle* particle1, Particle* particle2) {
    float dx = particle2->position[0] - particle1->position[0];
    float dy = particle2->position[1] - particle1->position[1];
    return sqrt(dx * dx + dy * dy);
}
