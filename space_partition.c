#include "space_partition.h"
#include "artist.h"
#include "arraylist.h"
#include <math.h>

void getSpacePartition(Node* particleNode) {
    /* 2 3
       0 1 */
    Particle* particle = (Particle*) particleNode->item;

    int partitionGridLength = (int) sqrt(spacePartitions);

    float x = particle->position[0];
    float y = particle->position[1];

    int partitionId = 0;

    float gridLength = box_length / partitionGridLength;
    partitionId = partitionId + x / gridLength;
    partitionId = partitionId + (int) (y / gridLength) * partitionGridLength;


    printf("%d\n", partitionId);
}