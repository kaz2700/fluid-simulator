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

### Bugs
- Collision detection only checks within each space partition, missing cross-partition collisions
- Static arrays in math functions (pointing_vector, normalized_vector) are not thread-safe
- translate_position() uses static array - may cause issues if called multiple times

### Features to Implement
- Viscosity
- Change coordinate system so origin (0,0) is at bottom left
- Memory management for particle removal
- Use proper vectors/arrays for position storage
- 3D support

### Improvements
- Make usleep() portable
- Check neighboring partitions for collisions
- Make math functions thread-safe
