#ifndef FORCES_H
#define FORCES_H

#include "core/particle.h"

extern float gravity_acceleration;

void apply_gravity(Particle* p);

#endif
