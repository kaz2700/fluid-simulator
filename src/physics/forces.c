#include "physics/forces.h"

float gravity_acceleration = 10.0f;

void apply_gravity(Particle* p) {
    p->acceleration[0] = 0;
    p->acceleration[1] = -gravity_acceleration;
}
