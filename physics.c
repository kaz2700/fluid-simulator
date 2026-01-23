#include "physics.h"
#include "particle.h"
#include "artist.h"
#include <stdio.h>
#include "math_functions.h"
#include "space_partition.h"
#include "arraylist.h"

float collision_energy_transmission = 1;
float collision_energy_transmission_walls = 0.95;
float gravity = 10;
float k = 1;

void update_positions(Node* particleNode, float dt) {
    Particle* particle = particleNode->item;
    Node* previous_space_partition = getSpacePartitionFromParticleNode(particleNode);


    particle->velocity[0] = particle->velocity[0] + particle->acceleration[0] * dt;
    particle->velocity[1] = particle->velocity[1] + particle->acceleration[1] * dt;

    check_limits(particle, dt);

    particle->position[0] = particle->position[0] + particle->velocity[0] * dt;
    particle->position[1] = particle->position[1] + particle->velocity[1] * dt;

    Node* new_space_partition = getSpacePartitionFromParticleNode(particleNode);
    if (previous_space_partition != new_space_partition)
        assignSpacePartition(particleNode, previous_space_partition, new_space_partition);
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
        // Only collide if particles are approaching each other
        // Calculate relative position and velocity
        float dx = particle2->position[0] - particle1->position[0];
        float dy = particle2->position[1] - particle1->position[1];
        float dvx = particle1->velocity[0] - particle2->velocity[0];
        float dvy = particle1->velocity[1] - particle2->velocity[1];
        
        // Dot product of relative velocity and relative position
        // If positive, particles are approaching; if negative, they're separating
        float approaching = dx * dvx + dy * dvy;
        
        if (approaching > 0) {
            collision(particle1, particle2);
        }
    }
}

void collision(Particle* particle1, Particle* particle2) {
    float v11 = particle1->velocity[0];
    float v12 = particle1->velocity[1];
    float v21 = particle2->velocity[0];
    float v22 = particle2->velocity[1];
    float m1 = particle1->mass;
    float m2 = particle2->mass;

    // Calculate relative velocity components
    float dvx = v11 - v21;
    float dvy = v12 - v22;
    
    // Calculate relative position components
    float dx = particle1->position[0] - particle2->position[0];
    float dy = particle1->position[1] - particle2->position[1];
    
    // Calculate collision response using proper elastic collision formula
    float dot_product = dvx * dx + dvy * dy;
    float distance_squared = dx * dx + dy * dy;
    
    if (distance_squared > 0) {
        float collision_scale = 2 * dot_product / ((m1 + m2) * distance_squared);
        
        particle1->velocity[0] = collision_energy_transmission * (v11 - m2 * collision_scale * dx);
        particle1->velocity[1] = collision_energy_transmission * (v12 - m2 * collision_scale * dy);
        particle2->velocity[0] = collision_energy_transmission * (v21 + m1 * collision_scale * dx);
        particle2->velocity[1] = collision_energy_transmission * (v22 + m1 * collision_scale * dy);
    }
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
    Node* currentSpacePartition = getSpacePartitionList();
    while (currentSpacePartition != NULL) {
        Node* particleNode = currentSpacePartition->item;
        while(particleNode != NULL) {
            Node* nextNode = particleNode->next;  // Save next BEFORE potential removal
            
            update_acceleration(particleNode, dt);
            update_positions(particleNode, dt);

            particleNode = nextNode;
        }
        currentSpacePartition = currentSpacePartition->next;
    }
}