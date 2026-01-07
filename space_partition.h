#include "arraylist.h"
#ifndef space_partition_header
#define space_partition_header

void assignSpacePartition(Node* item, Node* previous_space_partition, Node* new_space_partition);
Node* getSpacePartitionFromParticleNode(Node* particleNode);
void createSpacePartitions(int num_of_partitions);
Node* createParticleList(int num_of_particles);
Node* getSpacePartitionList();


#endif // ADDITIONAL_H