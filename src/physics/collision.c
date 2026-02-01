#include "physics/collision.h"
#include "core/math_utils.h"
#include "render/renderer.h"
#include <math.h>

float particle_restitution = 1.0f;
float wall_restitution = 0.95f;

void detect_and_resolve_collision(Particle* a, Particle* b, float dt) {
    if (distance_on_motion(a, b, dt) <= a->radius + b->radius) {
        float dx = b->position[0] - a->position[0];
        float dy = b->position[1] - a->position[1];
        float dvx = a->velocity[0] - b->velocity[0];
        float dvy = a->velocity[1] - b->velocity[1];
        float approaching = dx * dvx + dy * dvy;

        if (approaching > 0) {
            resolve_particle_collision(a, b);
        }
    }
}

void resolve_particle_collision(Particle* a, Particle* b) {
    float vax = a->velocity[0];
    float vay = a->velocity[1];
    float vbx = b->velocity[0];
    float vby = b->velocity[1];
    float ma = a->mass;
    float mb = b->mass;

    float dvx = vax - vbx;
    float dvy = vay - vby;
    float dx = a->position[0] - b->position[0];
    float dy = a->position[1] - b->position[1];

    float dot_product = dvx * dx + dvy * dy;
    float distance_squared = dx * dx + dy * dy;

    if (distance_squared > 0) {
        float collision_scale = 2 * dot_product / ((ma + mb) * distance_squared);

        a->velocity[0] = particle_restitution * (vax - mb * collision_scale * dx);
        a->velocity[1] = particle_restitution * (vay - mb * collision_scale * dy);
        b->velocity[0] = particle_restitution * (vbx + ma * collision_scale * dx);
        b->velocity[1] = particle_restitution * (vby + ma * collision_scale * dy);
    }
}

void handle_wall_collision(Particle* p, float dt) {
    float r = p->radius;

    if (p->position[1] + r + p->velocity[1] * dt >= domain_size && p->velocity[1] > 0)
        p->velocity[1] *= -wall_restitution;
    if (p->position[1] - r + p->velocity[1] * dt <= 0 && p->velocity[1] < 0)
        p->velocity[1] *= -wall_restitution;

    if (p->position[0] + r + p->velocity[0] * dt >= domain_size && p->velocity[0] > 0)
        p->velocity[0] *= -wall_restitution;
    if (p->position[0] - r + p->velocity[0] * dt <= 0 && p->velocity[0] < 0)
        p->velocity[0] *= -wall_restitution;
}
