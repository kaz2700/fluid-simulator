#include "space_partition.h"
#include "artist.h"
#include "arraylist.h"
#include <math.h>

Node* spacePartitionHeadNode;
Node* particleHeadNode;
int spacePartitions = 0;

Node* getSpacePartitionFromParticleNode(Node* particleNode) {
    /* 2 3
       0 1 */
        Particle* particle = particleNode->item;
        int partitionGridLength = (int) sqrt(spacePartitions);

        float x = particle->position[0];
        float y = particle->position[1];

        int partitionId = 0;

        float gridLength = box_length / partitionGridLength;
        partitionId = partitionId + x / gridLength;
        partitionId = partitionId + (int) (y / gridLength) * partitionGridLength;

        Node* spacePartition = getSpacePartitionList();
        int i = 0;
        while (i != partitionId) {
            spacePartition = spacePartition->next;
        }

        printf("%d\nPartitionID", partitionId);

        return spacePartition;
}

void addToSpacePartition(Node* spacePartition, Node* particleNode) {
    Node* partition = getSpacePartitionList();
    addToList((Node**) &spacePartition, particleNode);
}

void assignSpacePartition(Node* particleNode, Node* previous_space_partition, Node* new_space_partition) {
    addToSpacePartition(new_space_partition, particleNode);
    removeFromList(&previous_space_partition, particleNode);
}

void createSpacePartitions(int num_of_partitions) {
    spacePartitions = num_of_partitions;

    for (int i = 0; i < num_of_partitions; i++) {
        Node* newNode = malloc(sizeof(Node));
        newNode->item = NULL;
        newNode->next = NULL;
        addToList(&spacePartitionHeadNode, newNode);
    }
}

Node* createParticleList(int num_of_particles) {
    Particle original_particle;

    float x_init =  box_length/4;
    float y_init = box_length/4;

    original_particle.radius = 0.01;
    original_particle.mass = 10;
    original_particle.charge = 0.05;

    int i_max = 3;

    float x = x_init;
    float y = y_init;

    particleHeadNode = NULL;

    for(int i = 0; i < num_of_particles; i++) {
        Particle* particle = malloc(sizeof(Particle));
        *particle = original_particle;

        x = x_init + 2 * particle->radius * (i % i_max);
        y = y_init + 2 * particle->radius * (i / i_max);

        if (i % i_max == 0)
            x = x_init;

        particle->position[0] = x;
        particle->position[1] = y;
        particle->velocity[0] = (float) random() / RAND_MAX;
        particle->velocity[1] = (float) random() / RAND_MAX;
        printf("NEW PARTICLE\n");

        Node* particleNode = particleHeadNode->next;
        particleNode->item = particle;

        Node* spacePartitionNode = getSpacePartitionFromParticleNode(particleNode);

        addToSpacePartition(spacePartitionNode, particleNode);
    }
}

Node* getParticleList() {
    return particleHeadNode;
}

Node* getSpacePartitionList() {
    return spacePartitionHeadNode;
}