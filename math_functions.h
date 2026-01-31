#include "particle.h"

#ifndef math_functions_header
#define math_functions_header

float distance_on_motion(Particle* particle1, Particle* particle2, float dt);
float radial_velocity(Particle* particle1, Particle* particle2);
float* pointing_vector(Particle* particle1, Particle* particle2);
float* normalized_vector(float* vector);
float vector_norm(float* vector);
float distance(Particle* particle1, Particle* particle2);

#endif