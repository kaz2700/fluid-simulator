#include "physics.h"
#include "particle.h"
#include "artist.h"
#include <stdio.h>
#include "math_functions.h"
#include <math.h>

float collision_energy_transmission = 0.98;
float collision_energy_transmission_walls = 0.75;
float gravity = 10;
float k = 1;
float elastic_constant = 10000;

void update_positions(Particle* particle, float dt) {

    particle -> velocity[0] = particle -> velocity[0] + particle -> acceleration[0] * dt;
    particle -> velocity[1] = particle -> velocity[1] + particle -> acceleration[1] * dt;

    check_limits(particle, dt);

    particle -> position[0] = particle -> position[0] + particle -> velocity[0] * dt;
    particle -> position[1] = particle -> position[1] + particle -> velocity[1] * dt;
}

void check_limits(Particle* particle, float dt) {
    if (particle -> position[1] + particle -> radius + particle -> velocity[1] * dt >= box_length ||
        particle -> position[1] - particle -> radius + particle -> velocity[1] * dt  <= 0)
            particle -> velocity[1] = -1 * collision_energy_transmission_walls * particle -> velocity[1];

    if (particle -> position[0] + particle -> radius + particle -> velocity[0] * dt  >= box_length ||
        particle -> position[0] - particle -> radius + particle -> velocity[0] * dt  <= 0)
            particle -> velocity[0] = -1 * collision_energy_transmission_walls * particle -> velocity[0];
}

void check_collision(Particle* particle1, Particle* particle2, float dt) {
    if(distance_on_motion(particle1, particle2, dt) <= particle1 -> radius + particle2 -> radius) {
        float w = elastic_constant / particle1 -> mass;

        particle1 -> acceleration[0] += w * pointing_vector(particle1, particle2)[0] * (2 * particle1 -> radius - distance(particle1, particle2));
        particle1 -> acceleration[1] += w * pointing_vector(particle1, particle2)[1] * (2 * particle1 -> radius - distance(particle1, particle2));

        particle2 -> acceleration[0] += w * pointing_vector(particle2, particle1)[0] * (2 * particle1 -> radius - distance(particle1, particle2));
        particle2 -> acceleration[1] += w * pointing_vector(particle2, particle1)[1] * (2 * particle1 -> radius - distance(particle1, particle2));

        //collision(particle1, particle2);
    }
}

void collision(Particle* particle1, Particle* particle2) {
    float x11 = particle1 -> position[0];
    float x12 = particle1 -> position[1];
    float x21 = particle2 -> position[0];
    float x22 = particle2 -> position[1];
    float v11 = particle1 -> velocity[0];
    float v12 = particle1 -> velocity[1];
    float v21 = particle2 -> velocity[0];
    float v22 = particle2 -> velocity[1];
    float m1 = particle1 -> mass;
    float m2 = particle2 -> mass;

    particle1 -> velocity[0] = collision_energy_transmission * (v11 - 2*m2/(m1+m2) * (v11 - v21)); 
    particle1 -> velocity[1] = collision_energy_transmission * (v12 - 2*m2/(m1+m2) * (v12 - v22));
    particle2 -> velocity[0] = collision_energy_transmission * (v21 - 2*m1/(m1+m2) * (v21 - v11));
    particle2 -> velocity[1] = collision_energy_transmission * (v22 - 2*m1/(m1+m2) * (v22 - v12));
}

float* repulsion_force(Particle* particle1, Particle* particle2) {
    static float repulsion_force[2];

    float q1 = particle1 -> charge;
    float q2 = particle2 -> charge;

    float d = distance(particle1, particle2);
    
    float repulsion_force_norm = k * q1 * q2 / pow(d, 2);


    float cos = (particle1 -> position[0] - particle2 -> position[0])/d;
    float sin = (particle1 -> position[1] - particle2 -> position[1])/d;

    repulsion_force[0] = repulsion_force_norm * cos;
    repulsion_force[1] = repulsion_force_norm * sin;

    return repulsion_force;
    //change acceleration, add acceleration
}

void update_acceleration(Particle* particles[], int particle_index, int num_of_particles, float dt) {
    particles[particle_index] -> acceleration[0] = 0;
    particles[particle_index] -> acceleration[1] = -gravity;

    for(int i = 0; i < num_of_particles; i++) {
        if (particle_index == i)
            continue;    
        //todo not call twice
        //particles[particle_index] -> acceleration[0] = particles[particle_index] -> acceleration[0] + repulsion_force(particles[particle_index], particles[i])[0]/particles[particle_index] -> mass;
        //particles[particle_index] -> acceleration[1] = particles[particle_index] -> acceleration[1] + repulsion_force(particles[particle_index], particles[i])[1]/particles[particle_index] -> mass;
        check_collision(particles[particle_index], particles[i], dt);
    } 
}
    //todo add viscosity

void tick(Particle* particles[], float dt, int num_of_particles) {
    for(int i = 0; i < num_of_particles; i++) {
            update_acceleration(particles, i, num_of_particles, dt);
            update_positions(particles[i], dt);
        } 
}