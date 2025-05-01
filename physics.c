#include "physics.h"
#include "particle.h"
#include "artist.h"
#include <stdio.h>
#include "math_functions.h"
#include "space_partition.h"
#include "arraylist.h"

float collision_energy_transmission = 1;
float collision_energy_transmission_walls = 0.75;
float gravity = 0;
float k = 1;

void update_positions(Particle* particle, float dt) {

    particle->velocity[0] = particle->velocity[0] + particle->acceleration[0] * dt;
    particle->velocity[1] = particle->velocity[1] + particle->acceleration[1] * dt;

    check_limits(particle, dt);

    particle->position[0] = particle->position[0] + particle->velocity[0] * dt;
    particle->position[1] = particle->position[1] + particle->velocity[1] * dt;

    //"%f", particle->position[1]);

}

void check_limits(Particle* particle, float dt) {
    if (particle->position[1] + particle->radius + particle->velocity[1] * dt >= box_length ||
        particle->position[1] - particle->radius + particle->velocity[1] * dt  <= 0)
            particle->velocity[1] = -1 * collision_energy_transmission_walls * particle->velocity[1];

    if (particle->position[0] + particle->radius + particle->velocity[0] * dt  >= box_length ||
        particle->position[0] - particle->radius + particle->velocity[0] * dt  <= 0)
            particle->velocity[0] = -1 * collision_energy_transmission_walls * particle->velocity[0];
}

void check_collision(Particle* particle1, Particle* particle2, float dt) {
    if(distance_on_motion(particle1, particle2, dt) <= particle1->radius + particle2->radius) {
        collision(particle1, particle2);
    }
}

void collision(Particle* particle1, Particle* particle2) {
    float x11 = particle1->position[0];
    float x12 = particle1->position[1];
    float x21 = particle2->position[0];
    float x22 = particle2->position[1];
    float v11 = particle1->velocity[0];
    float v12 = particle1->velocity[1];
    float v21 = particle2->velocity[0];
    float v22 = particle2->velocity[1];
    float m1 = particle1->mass;
    float m2 = particle2->mass;

    particle1->velocity[0] = collision_energy_transmission * (v11 - 2*m2/(m1+m2) * (v11 - v21)); 
    particle1->velocity[1] = collision_energy_transmission * (v12 - 2*m2/(m1+m2) * (v12 - v22));
    particle2->velocity[0] = collision_energy_transmission * (v21 - 2*m1/(m1+m2) * (v21 - v11));
    particle2->velocity[1] = collision_energy_transmission * (v22 - 2*m1/(m1+m2) * (v22 - v12));
}

void update_acceleration(Node* current, float dt) {
    Particle* particle = (Particle*) current->item;
    particle->acceleration[0] = 0;
    particle->acceleration[1] = -gravity;

    Node* otherParticleNode = current->next;
    while(otherParticleNode != NULL) {
        if (current != otherParticleNode)
            check_collision(particle, (Particle*) otherParticleNode->item, dt);

        otherParticleNode = otherParticleNode->next;
    } 
}
    //todo add viscosity

void tick(float dt) {
    Node* current = getHeadNode();
    while (current != NULL) {
        update_acceleration(current, dt);
        update_positions(current->item, dt);
        getSpacePartition(current);


        current = current->next; //think, this could do more efficiency
    }
}