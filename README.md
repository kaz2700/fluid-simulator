# Fluid Simulator

A 2D particle-based fluid simulator written in C with SDL2 rendering.

## Overview

This simulator models fluid behavior using a particle system with:
- 500 particles with physical properties (position, velocity, acceleration, mass, radius, charge)
- Gravity and elastic collisions
- Spatial partitioning for optimized collision detection
- Real-time visualization with velocity-based coloring

## Building

```bash
make
```

## Running

```bash
./program
```

Or use the precompiled binary:

```bash
./a.out
```

## Controls

- Close the window to exit the simulation

## Architecture

### Core Components

**Particle System** (`particle.h`, `particle.c`)
- Each particle has position[2], velocity[2], acceleration[2], radius, mass, and charge
- Particles initialized in a centered grid pattern

**Physics Engine** (`physics.h`, `physics.c`)
- Semi-implicit Euler integration (velocity → position)
- Gravity: 10 m/s² downward
- Elastic particle-particle collisions with energy transmission = 1.0
- Wall collisions with energy loss = 0.95 restitution
- Time step: dt = 0.01 seconds (100 Hz simulation)

**Spatial Partitioning** (`space_partition.h`, `space_partition.c`)
- 4×4 grid (16 partitions) for collision optimization
- Particles dynamically move between partitions
- Reduces collision checks from O(n²) to near O(n)

**Rendering** (`artist.h`, `artist.c`)
- SDL2 window: 600×600 pixels
- Box size: 1 meter
- Particle coloring based on velocity (red = fast, white = slow)
- Pre-rendered particle textures for performance

**Linked List Utilities** (`arraylist.h`, `arraylist.c`)
- Custom linked list implementation using `Node` structures
- Functions: addToList, removeFromList, unlinkFromList, getFromList, getListLength

**Math Functions** (`math_functions.h`, `math_functions.c`)
- Distance calculations
- Vector operations (normalization, pointing vectors)
- Radial velocity computation

## Physics Model

- **Integration**: Semi-implicit Euler method
- **Collision Detection**: Predicts collision using `distance_on_motion()` 
- **Collision Response**: Elastic collision formula with mass consideration and dot product approach check
- **Wall Handling**: Reflective boundaries with energy loss

## TODO

### Critical Bugs
- [ ] **Partition recomputation bug**: `integrator.c` computes partition twice (always equal). Should compute once before position update, store it, then compare after.
- [ ] **Static array in `get_adjacent_partitions()`**: Returns pointer to static local array - thread-unsafe and risks data corruption. Should return by value or use arena allocation.
- [ ] **Neighbor search incomplete**: Only checks 4 neighbors (forward direction), missing half the adjacent partitions.
- [x] **Inefficient grid lookup**: `compute_partition_for_particle()` iterates linked list - O(n) instead of O(1) array access.

### Configuration & Architecture
- [ ] Centralize magic numbers (256 partitions, 0.005f radius, 10.0f gravity) in a `SimulationConfig` struct
- [ ] Encapsulate global state: `particle_restitution`, `wall_restitution`, `gravity_acceleration`, `domain_size` should be fields in a `PhysicsWorld` struct
- [ ] Create opaque handles for renderer (`Renderer*`) instead of exposing SDL globals
- [ ] Add simulation state object to hold `time_step`, `should_quit`, main loop state
- [ ] Fix tight coupling: Physics should not directly call renderer for wall bounds check

### API Design Improvements
- [ ] Clarify `apply_gravity()` contract - it zeroes acceleration then sets gravity. Should be `reset_acceleration()` then `apply_gravity()`.
- [ ] Add getter/setter functions for renderer instead of exposing `window`, `renderer`, `domain_size` as extern globals
- [ ] Fix const correctness: Mark read-only parameters as `const`

### Type Safety & Code Quality
- [ ] Replace void pointers in linked list with intrusive lists (embed Node in Particle) or macro-based generics
- [ ] Add unit tests for math functions, collision logic, grid indexing
- [ ] Remove dead code: `list_get_at()` never used, `charge` field in Particle unused
- [ ] Standardize naming: Some functions use `p`, others use `particle`
- [ ] Add documentation explaining coordinate systems, physics conventions, and algorithm choices
- [ ] Update AGENTS.md to reflect new file organization

### Memory Management
- [ ] Implement pool/arena allocator for particles and nodes (avoid 10,000 individual malloc/free calls)
- [ ] Fix cleanup on init failure: If `init_renderer()` fails after creating window, SDL isn't cleaned up properly
- [ ] Add proper error handling and resource cleanup paths

### Performance Optimizations
- [ ] Unify velocity/position passes: Two full loops over particles could be one for cache efficiency
- [ ] Switch from `SDL_RENDERER_SOFTWARE` to hardware acceleration
- [ ] Consider vertex-buffer based rendering instead of texture blitting for modern GPUs
- [ ] Evaluate double precision: Float precision may cause issues with 10k particles at 600px scale
- [ ] Pre-allocate grid array instead of linked list for O(1) partition access

### Features to Implement
- [ ] Viscosity for fluid-like behavior
- [ ] Memory management for particle removal (currently no deletion mechanism)
- [ ] 3D support
- [ ] Change coordinate system so origin (0,0) is at bottom left (physics convention)
- [ ] Use proper vector math libraries or SIMD for batch operations

### Portability
- [ ] Make `usleep()` portable (Windows doesn't support it)
- [ ] Add cross-platform build support (CMake or similar)
- [ ] Add CI/CD for automated testing on different platforms

### Testing & Documentation
- [ ] Add unit tests for math utilities (`vector_norm`, `distance_on_motion`)
- [ ] Add integration tests for collision detection
- [ ] Add benchmarks for spatial partitioning performance
- [ ] Write developer documentation for the physics model and algorithms
- [ ] Add inline comments explaining the collision response formula
- [ ] Document the semi-implicit Euler integration method and why it was chosen
