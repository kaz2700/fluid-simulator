#ifndef COLLISION_H
#define COLLISION_H

#include "core/particle.h"

extern float particle_restitution;
extern float wall_restitution;

void detect_and_resolve_collision(Particle* a, Particle* b, float dt);
void resolve_particle_collision(Particle* a, Particle* b);
void handle_wall_collision(Particle* p, float dt);

// Position-based constraint resolution
void resolve_position_overlaps(int max_iterations);
void enforce_position_constraints(void);
void clamp_particle_position(Particle* p);

// Collision pair caching for performance
typedef struct CollisionPair {
    Particle* a;
    Particle* b;
} CollisionPair;

void clear_collision_pairs(void);
void add_collision_pair(Particle* a, Particle* b);
void resolve_position_overlaps_cached(int max_iterations);

#endif
