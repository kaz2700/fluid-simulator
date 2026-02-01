#include "spatial/particle_factory.h"
#include "spatial/grid.h"
#include "render/renderer.h"
#include "core/particle.h"
#include "core/linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

void create_particles(int count) {
    Particle template = {
        .radius = 0.005f,
        .mass = 10.0f,
        .charge = 0.05f
    };

    int grid_dim = (int)ceil(sqrt(count));
    float spacing = 2 * template.radius;
    float grid_width = grid_dim * spacing;

    int max_per_row = (int)((domain_size - 2 * template.radius) / spacing) + 1;
    int max_particles = max_per_row * max_per_row;
    printf("Max particles that fit: %d (%d x %d grid)\n", max_particles, max_per_row, max_per_row);

    float x_init = (domain_size - grid_width) / 2 + template.radius;
    float y_init = (domain_size - grid_width) / 2 + template.radius;

    for (int i = 0; i < count; i++) {
        Particle* particle = malloc(sizeof(Particle));
        if (particle == NULL) {
            fprintf(stderr, "error: malloc failed for particle\n");
            exit(1);
        }
        *particle = template;

        int col = i % grid_dim;
        int row = i / grid_dim;
        float x = x_init + spacing * col;
        float y = y_init + spacing * row;

        particle->position[0] = x + ((float)rand() / RAND_MAX - 0.5f) * spacing * 0.5f;
        particle->position[1] = y + ((float)rand() / RAND_MAX - 0.5f) * spacing * 0.5f;
        particle->velocity[0] = (float)rand() / RAND_MAX;
        particle->velocity[1] = (float)rand() / RAND_MAX;

        Node* particle_node = malloc(sizeof(Node));
        if (particle_node == NULL) {
            fprintf(stderr, "error: malloc failed for particle_node\n");
            exit(1);
        }
        particle_node->item = particle;

        Node* partition_node = compute_partition_for_particle(particle_node);
        Node* particle_head = (Node*)partition_node->item;
        list_append(&particle_head, particle_node);
        partition_node->item = particle_head;
    }
}
