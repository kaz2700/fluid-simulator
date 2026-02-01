# AGENTS.md

Quick reference for coding agents working on this project.

## Project Type

2D particle-based fluid simulator written in C with SDL2 rendering. Models 10,000 particles with gravity, elastic collisions, and spatial partitioning.

## Build & Run

```bash
make          # Build
make run      # Build and run
./build/program     # Run
make clean    # Clean build artifacts
```

**Environment**: Uses Nix flake for dependencies (gcc, SDL2, SDL2_image, pkg-config, make).

## Directory Structure

```
.
├── src/                 # Source files (.c)
│   ├── main.c          # Entry point, main loop
│   ├── physics.c       # Physics engine, collision logic
│   ├── space_partition.c  # Grid-based spatial partitioning
│   ├── artist.c        # SDL2 rendering
│   ├── arraylist.c     # Linked list utilities
│   └── math_functions.c  # Vector math, distance calculations
├── include/            # Header files (.h)
│   ├── particle.h      # Particle struct definition
│   ├── physics.h       # Physics engine interface
│   ├── space_partition.h  # Spatial partitioning interface
│   ├── artist.h        # Rendering interface
│   ├── arraylist.h     # Linked list interface
│   └── math_functions.h  # Math functions interface
├── build/              # Build artifacts (generated)
│   ├── program         # Executable
│   └── *.o            # Object files
├── Makefile            # Build configuration
└── .gitignore          # Git ignore rules
```

## Core Architecture

### Data Structures

**Particle** (`include/particle.h`)
```c
typedef struct Particle {
    float position[2];    // x, y coordinates
    float velocity[2];    // vx, vy velocities
    float acceleration[2]; // ax, ay accelerations
    float radius;         // collision radius
    float mass;           // particle mass
    float charge;         // electric charge (unused)
} Particle;
```

**Node** (`include/arraylist.h`) - Singly linked list
```c
typedef struct Node {
    void* item;          // Generic pointer (Particle* or another Node*)
    struct Node* next;
} Node;
```

### Simulation Loop (src/main.c:24-36)

```c
while (!quit) {
    tick(dt);                           // Physics update
    draw();                             // Render
    usleep((useconds_t)(1000000 * dt));  // 100 Hz (dt = 0.01s)
}
```

### Physics Model

**Integration**: Semi-implicit Euler
1. Update velocity: `v = v + a * dt`
2. Check wall collisions
3. Update position: `x = x + v * dt`

**Constants** (src/physics.c:6-9)
- `gravity = 10` m/s² (downward)
- `collision_energy_transmission = 1.0` (elastic)
- `collision_energy_transmission_walls = 0.95` (5% energy loss)
- `dt = 0.01` seconds

**Collision Detection** (src/physics.c:33-51)
- Uses `distance_on_motion()` to predict collision
- Only collides if approaching (dot product check)
- Elastic collision response with mass consideration

**Spatial Partitioning** (src/space_partition.c:9-37)
- 16×16 grid (256 partitions)
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
- `createParticleList(int n)` - Initialize particles in centered grid (src/space_partition.c:96)
- `cleanupParticlesAndPartitions()` - Free all memory (src/space_partition.c:154)

### Physics Loop
- `tick(float dt)` - Main physics update (src/physics.c:110)
- `update_acceleration(Node*, float dt)` - Apply gravity, check collisions (src/physics.c:86)
- `check_collision(Particle*, Particle*, float dt)` - Collision detection (src/physics.c:33)
- `collision(Particle*, Particle*)` - Collision response (src/physics.c:53)

### Spatial Partitioning
- `createSpacePartitions(int n)` - Create grid (src/space_partition.c:44)
- `assignSpacePartition(Node*, Node*, Node*)` - Move particle between partitions (src/space_partition.c:41)
- `getSpacePartitionFromParticleNode(Node*)` - Find partition for particle (src/space_partition.c:9)
- `getNeighborPartitions(Node*)` - Get 8 neighboring partitions (src/space_partition.c:59)

### Linked List Utilities
- `addToList(Node**, Node*)` - Append to list (src/arraylist.c:19)
- `unlinkFromList(Node**, Node*)` - Remove WITHOUT freeing (for moving) (src/arraylist.c:67)
- `removeFromList(Node**, Node*)` - Remove and free (src/arraylist.c:34)
- `getListLength(Node*)` - Count nodes (src/arraylist.c:94)

### Rendering
- `init()` - Initialize SDL2 (src/artist.c:16)
- `draw()` - Render all particles (src/artist.c:63)
- `close_sdl()` - Cleanup SDL2 (src/artist.c:48)

## Known Bugs & Limitations

1. **Cross-partition collisions** - Only checks collisions within same partition and 8 neighbors (diagonals only in one direction)
2. **Thread-safety** - Static arrays in `getNeighborPartitions()` (src/space_partition.c:59)
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
    fprintf(stderr, "error: malloc failed\n");
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

## When Making Changes

- Always run `make` after changes
- Follow existing code style (4-space indentation, lowercase functions)
- Use `malloc()` with null checks, use `fprintf(stderr, ...)` for errors
- Update both `.h` and `.c` files for new functions
- Coordinate system: top-left origin (0,0), positive y = down
- Place new source files in `src/` and headers in `include/`
