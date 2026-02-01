#include "physics/integrator.h"
#include "physics/collision.h"
#include "physics/forces.h"
#include "spatial/grid.h"
#include "core/particle.h"
#include "core/linked_list.h"

static void update_acceleration(Node* current, float dt) {
    Particle* particle = (Particle*)current->item;
    apply_gravity(particle);

    Node* other_particle = current->next;
    while (other_particle != NULL) {
        detect_and_resolve_collision(particle, (Particle*)other_particle->item, dt);
        other_particle = other_particle->next;
    }

    Node* partition = compute_partition_for_particle(current);
    Node** neighbors = get_adjacent_partitions(partition);

    for (int i = 0; i < 8 && neighbors[i] != NULL; i++) {
        Node* neighbor_particle = (Node*)neighbors[i]->item;
        while (neighbor_particle != NULL) {
            detect_and_resolve_collision(particle, (Particle*)neighbor_particle->item, dt);
            neighbor_particle = neighbor_particle->next;
        }
    }
}

void physics_step(float time_step) {
    Node* current_partition = get_all_partitions();

    while (current_partition != NULL) {
        Node* particle_node = current_partition->item;
        while (particle_node != NULL) {
            Particle* particle = (Particle*)particle_node->item;

            particle->velocity[0] += particle->acceleration[0] * time_step;
            particle->velocity[1] += particle->acceleration[1] * time_step;

            update_acceleration(particle_node, time_step);

            particle_node = particle_node->next;
        }
        current_partition = current_partition->next;
    }

    current_partition = get_all_partitions();
    while (current_partition != NULL) {
        Node* particle_node = current_partition->item;
        while (particle_node != NULL) {
            Particle* particle = (Particle*)particle_node->item;

            particle->position[0] += particle->velocity[0] * time_step;
            particle->position[1] += particle->velocity[1] * time_step;

            handle_wall_collision(particle, time_step);

            Node* old_partition = compute_partition_for_particle(particle_node);
            Node* new_partition = compute_partition_for_particle(particle_node);
            if (old_partition != new_partition)
                move_particle_to_partition(particle_node, old_partition, new_partition);

            particle_node = particle_node->next;
        }
        current_partition = current_partition->next;
    }
}
