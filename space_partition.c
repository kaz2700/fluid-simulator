#include "space_partition.h"
#include "artist.h"
#include "arraylist.h"
#include <math.h>

int getSpacePartition(Particle* particle) {
    /* 2 3
       0 1 */
        int partitionGridLength = (int) sqrt(spacePartitions);

        float x = particle->position[0];
        float y = particle->position[1];

        int partitionId = 0;

        float gridLength = box_length / partitionGridLength;
        partitionId = partitionId + x / gridLength;
        partitionId = partitionId + (int) (y / gridLength) * partitionGridLength;


        printf("%d\n", partitionId);

        return partitionId;
}

void addToSpacePartition(int spacePartitionId, Particle* particle) {
    Node* partition = getSpacePartitionHeadNode();

    int i = 0;
    while (partition != NULL) {
        if (i == spacePartitionId) {
            addToLinkedList((Node**) &partition->item, particle);
            return;
        }

        i++;
        partition = partition->next;
    }

    printf("%d wtf not adding to space partition\n", spacePartitionId);
    return;
}

void assignSpacePartition(Particle* particle) {
    addToSpacePartition(getSpacePartition(particle), particle);
}