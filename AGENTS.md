# AGENTS.md

Quick reference for coding agents working on this project.

## Project Type

2D particle-based fluid simulator written in C with SDL2 rendering. Models 500 particles with gravity, elastic collisions, and spatial partitioning.

## Build & Run

```bash
make          # Build
./program     # Run
make clean    # Clean build artifacts
```

**Environment**: Uses Nix flake for dependencies (gcc, SDL2, SDL2_image, pkg-config, make).

## Core Architecture

### Data Structures

**Particle** (`particle.h/c`)
```c
typedef struct Particle {
    float position[2];    // x, y coordinates
    float velocity[2];    // vx, vy velocities
    float acceleration[2]; // ax, ay accelerations
    float radius;        // collision radius
    float mass;          // particle mass
    float charge;        // electric charge (unused)
}
```

**Node** (`arraylist.h/c`) - Singly linked list
```c
typedef struct Node {
    void* item;          // Generic pointer (Particle* or another Node*)
    struct Node* next;
}
```

### Simulation Loop (main.c:32-35)

```
while (!quit) {
    tick(dt);      // Physics update
    draw();        // Render
    usleep(1000000 * dt);  // 100 Hz (dt = 0.01s)
}
```

### Physics Model

**Integration**: Semi-implicit Euler
1. Update velocity: `v = v + a * dt`
2. Check wall collisions
3. Update position: `x = x + v * dt`

**Constants** (physics.c:9-12)
- `gravity = 10` m/s² (downward)
- `collision_energy_transmission = 1.0` (elastic)
- `collision_energy_transmission_walls = 0.95` (5% energy loss)
- `dt = 0.01` seconds

**Collision Detection** (physics.c:44-61)
- Uses `distance_on_motion()` to predict collision
- Only collides if approaching (dot product check)
- Elastic collision response with mass consideration

**Spatial Partitioning** (space_partition.c:9-39)
- 4×4 grid (16 partitions)
- Partition ID = `floor(x / gridLength) + floor(y / gridLength) * partitionGridLength`
- Particles move between partitions dynamically
- Grid cells store linked lists of particle nodes

### Rendering

**Coordinate System**: Origin at top-left (y increases downward)
- Window: 600×600 pixels
- Box size: 1 meter (scale: 600 px/m)
- Particle color: based on velocity (red = fast, white = slow)
- Pre-rendered textures for performance

## Key Functions

### Particle Management
- `createParticleList(int n)` - Initialize particles in centered grid
- `cleanupParticlesAndPartitions()` - Free all memory

### Physics Loop
- `tick(float dt)` - Main physics update (physics.c:106-119)
- `update_positions(Node*, float dt)` - Velocity/position integration
- `update_acceleration(Node*, float dt)` - Apply gravity, check collisions
- `check_collision(Particle*, Particle*, float dt)` - Collision detection
- `collision(Particle*, Particle*)` - Collision response

### Spatial Partitioning
- `createSpacePartitions(int n)` - Create grid
- `assignSpacePartition(Node*, Node*, Node*)` - Move particle between partitions
- `getSpacePartitionFromParticleNode(Node*)` - Find partition for particle

### Linked List Utilities
- `addToList(Node**, Node*)` - Append to list
- `unlinkFromList(Node**, Node*)` - Remove WITHOUT freeing (for moving)
- `removeFromList(Node**, Node*)` - Remove and free
- `getListLength(Node*)` - Count nodes

## Known Bugs & Limitations

1. **Cross-partition collisions** - Only checks collisions within same partition, not neighbors
2. **Thread-safety** - Static arrays in `pointing_vector()`, `normalized_vector()`, `translate_position()`
3. **Coordinate origin** - Top-left instead of bottom-left (physics convention)
4. **Memory leaks** - No particle removal mechanism during simulation

## Code Patterns

### Linked List Iteration
```c
Node* current = headNode;
while (current != NULL) {
    // Process current->item
    current = current->next;
}
```

### Node Allocation
```c
Node* newNode = malloc(sizeof(Node));
if (newNode == NULL) {
    printf("error: malloc failed\n");
    exit(1);
}
newNode->item = data;
newNode->next = NULL;
addToList(&headNode, newNode);
```

### Physics Update Order
1. Create space partitions
2. Create particles (automatically assigned to partitions)
3. For each partition: update acceleration, update position, reassign partition if moved

## File Structure

| File | Purpose |
|------|---------|
| `main.c` | Entry point, main loop |
| `particle.h/c` | Particle struct definition |
| `physics.h/c` | Physics engine, collision logic |
| `space_partition.h/c` | Grid-based spatial partitioning |
| `artist.h/c` | SDL2 rendering |
| `arraylist.h/c` | Linked list utilities |
| `math_functions.h/c` | Vector math, distance calculations |

## When Making Changes

- Always run `make` after changes
- Follow existing code style (4-space indentation, lowercase functions)
- Use `malloc()` with null checks
- Update both `.h` and `.c` files for new functions
- Coordinate system: top-left origin (0,0), positive y = down
