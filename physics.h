#include "particle.h"
#include "arraylist.h"

#ifndef physics_header
#define physics_header

//float gravity_value = 9.81;

extern float collision_energy_transmission;
extern float gravity;
extern float k;

void update_positions(Particle* particle, float dt);
void check_limits(Particle* particle, float dt);
void check_collision(Particle* particle1, Particle* particle2, float dt);
void collision(Particle* particle1, Particle* particle2);
void update_acceleration(Node* current, float dt);
void repulsion(Particle* particle1, Particle* particle2, float dt);
void tick(float dt);
#endif