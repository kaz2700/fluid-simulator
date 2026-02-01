#include "physics/collision.h"
#include "core/math_utils.h"
#include "render/renderer.h"
#include "spatial/grid.h"
#include "core/linked_list.h"
#include <math.h>

float particle_restitution = 1.0f;
float wall_restitution = 0.95f;

// Position-based constraint parameters
static const float position_correction_fraction = 0.5f;  // How much to correct per iteration (0-1)
static const float min_penetration_threshold = 0.0001f;   // Stop iterating when max penetration is below this

// Collision pair cache for eliminating redundant spatial queries
#define MAX_COLLISION_PAIRS 50000
static CollisionPair collision_pair_cache[MAX_COLLISION_PAIRS];
static int collision_pair_count = 0;

void detect_and_resolve_collision(Particle* a, Particle* b, float dt) {
    if (distance_on_motion(a, b, dt) <= a->radius + b->radius) {
        float dx = b->position[0] - a->position[0];
        float dy = b->position[1] - a->position[1];
        float dvx = a->velocity[0] - b->velocity[0];
        float dvy = a->velocity[1] - b->velocity[1];
        float approaching = dx * dvx + dy * dvy;

        if (approaching > 0) {
            resolve_particle_collision(a, b);
            // Cache this pair for position resolution phase
            add_collision_pair(a, b);
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

    // Predictive velocity reflection (existing behavior)
    if (p->position[1] + r + p->velocity[1] * dt >= domain_size && p->velocity[1] > 0)
        p->velocity[1] *= -wall_restitution;
    if (p->position[1] - r + p->velocity[1] * dt <= 0 && p->velocity[1] < 0)
        p->velocity[1] *= -wall_restitution;

    if (p->position[0] + r + p->velocity[0] * dt >= domain_size && p->velocity[0] > 0)
        p->velocity[0] *= -wall_restitution;
    if (p->position[0] - r + p->velocity[0] * dt <= 0 && p->velocity[0] < 0)
        p->velocity[0] *= -wall_restitution;
}

void clamp_particle_position(Particle* p) {
    float r = p->radius;
    
    // Hard position clamping to prevent escape
    if (p->position[0] < r) {
        p->position[0] = r;
        if (p->velocity[0] < 0) p->velocity[0] = 0;
    }
    if (p->position[0] > domain_size - r) {
        p->position[0] = domain_size - r;
        if (p->velocity[0] > 0) p->velocity[0] = 0;
    }
    if (p->position[1] < r) {
        p->position[1] = r;
        if (p->velocity[1] < 0) p->velocity[1] = 0;
    }
    if (p->position[1] > domain_size - r) {
        p->position[1] = domain_size - r;
        if (p->velocity[1] > 0) p->velocity[1] = 0;
    }
}

static void resolve_pair_overlap(Particle* a, Particle* b) {
    float dx = b->position[0] - a->position[0];
    float dy = b->position[1] - a->position[1];
    float distance_squared = dx * dx + dy * dy;
    float min_distance = a->radius + b->radius;
    
    if (distance_squared < min_distance * min_distance && distance_squared > 0) {
        float distance = sqrtf(distance_squared);
        float penetration = min_distance - distance;
        
        // Normal vector
        float nx = dx / distance;
        float ny = dy / distance;
        
        // Position correction (proportional to inverse mass)
        float total_mass = a->mass + b->mass;
        float a_ratio = b->mass / total_mass;
        float b_ratio = a->mass / total_mass;
        
        float correction = penetration * position_correction_fraction;
        
        a->position[0] -= nx * correction * a_ratio;
        a->position[1] -= ny * correction * a_ratio;
        b->position[0] += nx * correction * b_ratio;
        b->position[1] += ny * correction * b_ratio;
    }
}

void resolve_position_overlaps(int max_iterations) {
    for (int iteration = 0; iteration < max_iterations; iteration++) {
        float max_penetration = 0.0f;
        int corrections_made = 0;
        
        Node* current_partition = get_all_partitions();
        while (current_partition != NULL) {
            Node* particle_node = current_partition->item;
            while (particle_node != NULL) {
                Particle* particle = (Particle*)particle_node->item;
                
                // Check against other particles in same partition
                Node* other = particle_node->next;
                while (other != NULL) {
                    Particle* other_particle = (Particle*)other->item;
                    
                    float dx = other_particle->position[0] - particle->position[0];
                    float dy = other_particle->position[1] - particle->position[1];
                    float dist_sq = dx * dx + dy * dy;
                    float min_dist = particle->radius + other_particle->radius;
                    
                    if (dist_sq < min_dist * min_dist && dist_sq > 0) {
                        float dist = sqrtf(dist_sq);
                        float penetration = min_dist - dist;
                        if (penetration > max_penetration) {
                            max_penetration = penetration;
                        }
                        resolve_pair_overlap(particle, other_particle);
                        corrections_made++;
                    }
                    other = other->next;
                }
                
                // Check against particles in adjacent partitions
                Node** neighbors = get_adjacent_partitions(current_partition);
                for (int i = 0; i < 8 && neighbors[i] != NULL; i++) {
                    Node* neighbor_particle = (Node*)neighbors[i]->item;
                    while (neighbor_particle != NULL) {
                        Particle* np = (Particle*)neighbor_particle->item;
                        
                        float dx = np->position[0] - particle->position[0];
                        float dy = np->position[1] - particle->position[1];
                        float dist_sq = dx * dx + dy * dy;
                        float min_dist = particle->radius + np->radius;
                        
                        if (dist_sq < min_dist * min_dist && dist_sq > 0) {
                            float dist = sqrtf(dist_sq);
                            float penetration = min_dist - dist;
                            if (penetration > max_penetration) {
                                max_penetration = penetration;
                            }
                            resolve_pair_overlap(particle, np);
                            corrections_made++;
                        }
                        neighbor_particle = neighbor_particle->next;
                    }
                }
                
                particle_node = particle_node->next;
            }
            current_partition = current_partition->next;
        }
        
        // Early termination if no significant overlaps remain
        if (max_penetration < min_penetration_threshold || corrections_made == 0) {
            break;
        }
    }
}

void enforce_position_constraints(void) {
    Node* current_partition = get_all_partitions();
    while (current_partition != NULL) {
        Node* particle_node = current_partition->item;
        while (particle_node != NULL) {
            Particle* particle = (Particle*)particle_node->item;
            clamp_particle_position(particle);
            particle_node = particle_node->next;
        }
        current_partition = current_partition->next;
    }
}

// Collision pair cache management
void clear_collision_pairs(void) {
    collision_pair_count = 0;
}

void add_collision_pair(Particle* a, Particle* b) {
    if (collision_pair_count < MAX_COLLISION_PAIRS) {
        collision_pair_cache[collision_pair_count].a = a;
        collision_pair_cache[collision_pair_count].b = b;
        collision_pair_count++;
    }
}

// Cached version: uses pre-computed collision pairs instead of spatial queries
void resolve_position_overlaps_cached(int max_iterations) {
    if (collision_pair_count == 0) return;
    
    for (int iteration = 0; iteration < max_iterations; iteration++) {
        float max_penetration = 0.0f;
        int corrections_made = 0;
        
        // Use cached pairs - no spatial queries needed!
        for (int i = 0; i < collision_pair_count; i++) {
            Particle* a = collision_pair_cache[i].a;
            Particle* b = collision_pair_cache[i].b;
            
            // Quick squared distance check
            float dx = b->position[0] - a->position[0];
            float dy = b->position[1] - a->position[1];
            float dist_sq = dx * dx + dy * dy;
            float min_dist = a->radius + b->radius;
            
            if (dist_sq < min_dist * min_dist && dist_sq > 0.000001f) {
                float dist = sqrtf(dist_sq);
                float penetration = min_dist - dist;
                
                if (penetration > max_penetration) {
                    max_penetration = penetration;
                }
                
                // Normal vector
                float nx = dx / dist;
                float ny = dy / dist;
                
                // Position correction (proportional to inverse mass)
                float total_mass = a->mass + b->mass;
                float a_ratio = b->mass / total_mass;
                float b_ratio = a->mass / total_mass;
                
                float correction = penetration * position_correction_fraction;
                
                a->position[0] -= nx * correction * a_ratio;
                a->position[1] -= ny * correction * a_ratio;
                b->position[0] += nx * correction * b_ratio;
                b->position[1] += ny * correction * b_ratio;
                
                corrections_made++;
            }
        }
        
        // Early termination if no significant overlaps remain
        if (max_penetration < min_penetration_threshold || corrections_made == 0) {
            break;
        }
    }
}
