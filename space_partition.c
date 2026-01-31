#include "space_partition.h"
#include "artist.h"
#include "arraylist.h"
#include <math.h>

Node* spacePartitionHeadNode;
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
        partitionId = partitionId + (int)(x / gridLength);
        partitionId = partitionId + (int)(y / gridLength) * partitionGridLength;
        
        if (partitionId >= spacePartitions) {
            partitionId = spacePartitions - 1;
        }
        if (partitionId < 0) {
            partitionId = 0;
        }

        Node* spacePartition = getSpacePartitionList();
        int i = 0;
        while (i != partitionId) {
            spacePartition = spacePartition->next;
            i++;
        }

        return spacePartition;
}

void assignSpacePartition(Node* particleNode, Node* previous_space_partition, Node* new_space_partition) {
    unlinkFromList((Node**)&previous_space_partition->item, particleNode);
    addToList((Node**)&new_space_partition->item, particleNode);
}

void createSpacePartitions(int num_of_partitions) {
    spacePartitions = num_of_partitions;

    for (int i = 0; i < num_of_partitions; i++) {
        Node* newNode = malloc(sizeof(Node));
        if (newNode == NULL) {
            printf("error: malloc failed for space partition\n");
            exit(1);
        }
        newNode->item = NULL;
        newNode->next = NULL;
        addToList(&spacePartitionHeadNode, newNode);
    }
}

Node* createParticleList(int num_of_particles) {
    Particle original_particle;

    original_particle.radius = 0.01;
    original_particle.mass = 10;
    original_particle.charge = 0.05;

    // Calculate grid dimensions - square grid centered in box
    int grid_size = (int) ceil(sqrt(num_of_particles));  // particles per row
    float spacing = 2 * original_particle.radius;
    float grid_width = grid_size * spacing;
    
    // Calculate and print max particles that fit in the box
    // Account for particle radius: first particle center is at radius from edge,
    // last particle center must be at least radius from opposite edge
    int max_per_row = (int)((box_length - 2 * original_particle.radius) / spacing) + 1;
    int max_particles = max_per_row * max_per_row;
    printf("Max particles that fit: %d (%d x %d grid)\n", max_particles, max_per_row, max_per_row);
    
    // Center the grid in the box
    float x_init = (box_length - grid_width) / 2 + original_particle.radius;
    float y_init = (box_length - grid_width) / 2 + original_particle.radius;

    for(int i = 0; i < num_of_particles; i++) {
        Particle* particle = malloc(sizeof(Particle));
        if (particle == NULL) {
            printf("error: malloc failed for particle\n");
            exit(1);
        }
        *particle = original_particle;

        int col = i % grid_size;
        int row = i / grid_size;
        float x = x_init + spacing * col;
        float y = y_init + spacing * row;

        particle->position[0] = x;
        particle->position[1] = y;
        particle->velocity[0] = (float) random() / RAND_MAX;
        particle->velocity[1] = (float) random() / RAND_MAX;

        Node* particleNode = malloc(sizeof(Node));
        if (particleNode == NULL) {
            printf("error: malloc failed for particleNode\n");
            exit(1);
        }
        particleNode->item = particle;

        Node* spacePartitionNode = getSpacePartitionFromParticleNode(particleNode);

        Node* particleHeadNode = (Node*) spacePartitionNode->item;
        addToList(&particleHeadNode, particleNode);
        spacePartitionNode->item = particleHeadNode;
    }
}

Node* getSpacePartitionList() {
    return spacePartitionHeadNode;
}

void cleanupParticlesAndPartitions() {
    Node* spacePartitionNode = spacePartitionHeadNode;
    while (spacePartitionNode != NULL) {
        Node* particleNode = spacePartitionNode->item;
        while (particleNode != NULL) {
            Node* nextParticleNode = particleNode->next;
            free(particleNode->item);
            free(particleNode);
            particleNode = nextParticleNode;
        }
        spacePartitionNode->item = NULL;
        spacePartitionNode = spacePartitionNode->next;
    }
    
    spacePartitionNode = spacePartitionHeadNode;
    while (spacePartitionNode != NULL) {
        Node* nextSpacePartitionNode = spacePartitionNode->next;
        free(spacePartitionNode);
        spacePartitionNode = nextSpacePartitionNode;
    }
    
    spacePartitionHeadNode = NULL;
    spacePartitions = 0;
}