#include "spatial/grid.h"
#include "render/renderer.h"
#include "core/particle.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static Node* partition_list = NULL;
static int num_partitions = 0;

Node* compute_partition_for_particle(Node* particle_node) {
    Particle* particle = particle_node->item;
    int grid_dim = (int)sqrt(num_partitions);
    float cell_size = domain_size / grid_dim;
    int partition_id = (int)(particle->position[0] / cell_size) +
                       (int)(particle->position[1] / cell_size) * grid_dim;

    if (partition_id >= num_partitions)
        partition_id = num_partitions - 1;
    if (partition_id < 0)
        partition_id = 0;

    Node* partition = get_all_partitions();
    for (int i = 0; i < partition_id && partition != NULL; i++)
        partition = partition->next;

    return partition;
}

void move_particle_to_partition(Node* particle_node, Node* old_partition, Node* new_partition) {
    list_unlink((Node**)&old_partition->item, particle_node);
    list_append((Node**)&new_partition->item, particle_node);
}

void init_grid(int num_parts) {
    num_partitions = num_parts;

    for (int i = 0; i < num_parts; i++) {
        Node* new_node = malloc(sizeof(Node));
        if (new_node == NULL) {
            fprintf(stderr, "error: malloc failed for space partition\n");
            exit(1);
        }
        new_node->item = NULL;
        new_node->next = NULL;
        list_append(&partition_list, new_node);
    }
}

Node** get_adjacent_partitions(Node* partition_node) {
    static Node* neighbors[8];
    int count = 0;

    Node* current = get_all_partitions();
    int partition_id = 0;
    while (current != partition_node && current != NULL) {
        current = current->next;
        partition_id++;
    }

    int grid_dim = (int)sqrt(num_partitions);
    int x = partition_id % grid_dim;
    int y = partition_id / grid_dim;

    for (int dy = 0; dy <= 1; dy++) {
        int dx_start = (dy == 0) ? 1 : 0;
        for (int dx = dx_start; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < grid_dim && ny >= 0 && ny < grid_dim) {
                int neighbor_id = nx + ny * grid_dim;
                Node* neighbor = get_all_partitions();
                for (int i = 0; i < neighbor_id && neighbor != NULL; i++)
                    neighbor = neighbor->next;
                if (neighbor != NULL)
                    neighbors[count++] = neighbor;
            }
        }
    }

    while (count < 8)
        neighbors[count++] = NULL;

    return neighbors;
}

Node* get_all_partitions(void) {
    return partition_list;
}

int get_partition_count(void) {
    return num_partitions;
}

void cleanup_grid(void) {
    Node* partition_node = partition_list;
    while (partition_node != NULL) {
        Node* particle_node = partition_node->item;
        while (particle_node != NULL) {
            Node* next_particle = particle_node->next;
            free(particle_node->item);
            free(particle_node);
            particle_node = next_particle;
        }
        partition_node->item = NULL;
        partition_node = partition_node->next;
    }

    partition_node = partition_list;
    while (partition_node != NULL) {
        Node* next_partition = partition_node->next;
        free(partition_node);
        partition_node = next_partition;
    }

    partition_list = NULL;
    num_partitions = 0;
}
