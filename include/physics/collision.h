#ifndef COLLISION_H
#define COLLISION_H

#include "core/particle.h"

extern float particle_restitution;
extern float wall_restitution;

void detect_and_resolve_collision(Particle* a, Particle* b, float dt);
void resolve_particle_collision(Particle* a, Particle* b);
void handle_wall_collision(Particle* p, float dt);

#endif
