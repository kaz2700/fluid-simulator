#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "core/particle.h"

float distance_on_motion(Particle* particle1, Particle* particle2, float dt);
void pointing_vector(Particle* particle1, Particle* particle2, float* result);
void normalized_vector(const float* vector, float* result);
float vector_norm(const float* vector);
float distance(Particle* particle1, Particle* particle2);

#endif
