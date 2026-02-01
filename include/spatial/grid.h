#ifndef GRID_H
#define GRID_H

#include "core/linked_list.h"

void init_grid(int num_partitions);
void cleanup_grid(void);
Node* get_all_partitions(void);
Node* compute_partition_for_particle(Node* particle_node);
void move_particle_to_partition(Node* particle_node, Node* old_partition, Node* new_partition);
Node** get_adjacent_partitions(Node* partition);
int get_partition_count(void);

#endif
