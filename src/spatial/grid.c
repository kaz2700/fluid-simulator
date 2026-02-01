#include "spatial/grid.h"
#include "render/renderer.h"
#include "core/particle.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static Node* partition_list = NULL;
static Node** partition_array = NULL;  // O(1) lookup array
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

    return partition_array[partition_id];  // O(1) array access
}

void move_particle_to_partition(Node* particle_node, Node* old_partition, Node* new_partition) {
    list_unlink((Node**)&old_partition->item, particle_node);
    list_append((Node**)&new_partition->item, particle_node);
}

void init_grid(int num_parts) {
    num_partitions = num_parts;

    // Allocate O(1) lookup array
    partition_array = malloc(num_parts * sizeof(Node*));
    if (partition_array == NULL) {
        fprintf(stderr, "error: malloc failed for partition array\n");
        exit(1);
    }

    for (int i = 0; i < num_parts; i++) {
        Node* new_node = malloc(sizeof(Node));
        if (new_node == NULL) {
            fprintf(stderr, "error: malloc failed for space partition\n");
            exit(1);
        }
        new_node->item = NULL;
        new_node->next = NULL;
        list_append(&partition_list, new_node);
        partition_array[i] = new_node;  // Store in array for O(1) access
    }
}

Node** get_adjacent_partitions(Node* partition_node) {
    static Node* neighbors[8];
    int count = 0;

    // Find partition_id using array (O(n) but only once, not per neighbor)
    int partition_id = -1;
    for (int i = 0; i < num_partitions; i++) {
        if (partition_array[i] == partition_node) {
            partition_id = i;
            break;
        }
    }
    if (partition_id == -1) {
        while (count < 8)
            neighbors[count++] = NULL;
        return neighbors;
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
                neighbors[count++] = partition_array[neighbor_id];  // O(1) array access
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
    
    free(partition_array);
    partition_array = NULL;
    num_partitions = 0;
}
