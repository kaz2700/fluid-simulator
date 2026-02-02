# SPH 2D Simulator Implementation Plan

## Project Context

### Overview
This project implements a 2D Smoothed Particle Hydrodynamics (SPH) fluid simulation system. SPH is a Lagrangian (particle-based) method for simulating fluid dynamics, where fluid is represented by discrete particles that carry physical properties (position, velocity, density, pressure, etc.). The simulation computes inter-particle forces based on smoothing kernel functions to model fluid behavior.

### User Requirements
- **Language/Framework**: C++ + OpenGL
- **Primary Focus**: Performance optimization
- **Experience Level**: Advanced (user has experience with physics simulations)

### Development Philosophy
This simulator is built **incrementally** - each phase adds functionality that can be tested and verified independently before proceeding to the next phase. This approach ensures:
1. Early detection of issues (rendering, physics, or performance)
2. Progressive complexity - start simple, add features systematically
3. Each phase produces a working, testable state
4. Performance bottlenecks are identified and addressed throughout development

### Key Technical Goals
1. **Performance**: Handle 10,000+ particles at 60 FPS on modern hardware
2. **Accuracy**: Implement physically correct SPH equations with proper kernels
3. **Stability**: Use numerical methods that conserve energy and prevent instabilities
4. **Extensibility**: Code structure should allow easy addition of new features

### Performance-Critical Decisions
The following decisions were made specifically for performance:
- **Structure of Arrays (SoA)** memory layout: Improves cache locality and enables SIMD vectorization
- **Uniform grid spatial partitioning**: Reduces neighbor search from O(n²) to O(n)
- **Instanced rendering**: Draw thousands of particles efficiently
- **Pre-allocated memory**: Avoid dynamic allocations during simulation
- **Fixed-size grid cells**: Eliminates per-frame allocations for neighbor lists

### Simulation Parameters (Tunable)
- **Smoothing length (h)**: Interaction radius between particles (typically 2-3x particle spacing)
- **Particle mass**: Determines fluid density and pressure response
- **Rest density (ρ₀)**: Target fluid density for pressure calculation
- **Stiffness (B)**: Pressure constant in equation of state (controls incompressibility)
- **Viscosity (μ)**: Controls fluid thickness and damping
- **Time step (dt)**: Simulation timestep, may be adaptive based on CFL condition

### Dependencies
- **GLFW**: Window and input management
- **GLAD**: OpenGL function loader
- **GLM**: OpenGL mathematics library (vectors, matrices)
- **CMake**: Build system

### Current Status
**Phase**: Pre-implementation (planning complete)

### How to Use This Plan
1. **Read the current phase** carefully before starting
2. **Implement each step in order** - they build upon each other
3. **Test after each phase** using the testing strategy below
4. **Mark completed phases** by updating this document
5. **Feel free to adjust** specific implementation details as long as design goals are met

### Working on This Project
When continuing development on this project:
1. Check which phase was last completed
2. Review the next phase's steps
3. Implement the next step incrementally
4. Test thoroughly before moving to the next step
5. Update this PLAN.md with any deviations or improvements

---

## Phase 1: Project Setup & Basic Rendering

### Step 1.1: Initialize CMake Project
- Create CMakeLists.txt with minimum C++17 standard
- Set up GLFW and GLAD dependencies
- Configure build system for target platform

### Step 1.2: OpenGL Context Creation
- Initialize GLFW window
- Create OpenGL context (3.3+ core profile)
- Set up GLAD loader
- Configure viewport and basic OpenGL state

### Step 1.3: Particle Data Structure (SoA)
- Create Particle class with position, velocity, acceleration
- Use Structure of Arrays layout for cache efficiency
```cpp
struct Particles {
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;
    std::vector<glm::vec2> accelerations;
    std::vector<float> densities;
    std::vector<float> pressures;
};
```

### Step 1.4: Basic Shaders
- Create vertex shader for particle rendering
- Create fragment shader for particle coloring
- Compile and link shader program

### Step 1.5: Particle Rendering System
- Create vertex array object (VAO) and vertex buffer objects (VBOs)
- Implement instanced rendering for performance
- Render static particles as test

### Step 1.6: Main Loop Structure
- Set up game loop with delta time
- Implement clear and swap buffers
- Add FPS counter

---

## Phase 2: Particle Movement

### Step 2.1: Time Integration System
- Implement Velocity Verlet integrator:
  ```cpp
  v(t+0.5*dt) = v(t) + 0.5 * a(t) * dt
  x(t+dt) = x(t) + v(t+0.5*dt) * dt
  // Compute new accelerations
  v(t+dt) = v(t+0.5*dt) + 0.5 * a(t+dt) * dt
  ```

### Step 2.2: Initial Particle Configuration
- Create function to spawn particles in grid pattern
- Set initial velocities (can be zero)

### Step 2.3: Boundary Handling
- Implement reflective boundary conditions:
  - Detect collision with domain edges
  - Reflect velocity component perpendicular to wall
  - Apply damping factor for energy loss

### Step 2.4: Basic Motion Test
- Give particles random initial velocities
- Verify they move correctly and bounce off walls

---

## Phase 3: Spatial Partitioning (Performance Critical)

### Step 3.1: Uniform Grid Structure
- Design grid cell size based on smoothing length h
- Implement spatial hash function:
  ```cpp
  int cellX = floor((x - originX) / cellSize);
  int cellY = floor((y - originY) / cellSize);
  ```

### Step 3.2: Particle-Cell Mapping
- Create 2D grid array storing particle indices
- Implement update function to rebuild grid each frame
- Consider using fixed-size arrays per cell to avoid allocations

### Step 3.3: Neighbor Search
- Implement function to find neighbors within smoothing length h:
  - Query grid cells within ±1 cell radius
- Return list of neighbor indices

### Step 3.4: Performance Testing
- Benchmark neighbor search with 1000, 5000, 10000 particles
- Compare O(n²) vs O(n) neighbor search performance

---

## Phase 4: SPH Core Physics

### Step 4.1: Smoothing Kernel Functions
- Implement Poly6 kernel for density:
  ```
  W_poly6(r, h) = 315/(64*π*h⁹) * (h² - r²)³ for r ≤ h
  ```
- Implement Spiky kernel gradient for pressure forces:
  ```
  ∇W_spiky(r, h) = -45/(π*h⁶) * (h - r)² * (r/|r|) for r ≤ h
  ```
- Implement Laplacian kernel for viscosity:
  ```
  ∇²W_viscosity(r, h) = 45/(π*h⁶) * (h - r) for r ≤ h
  ```

### Step 4.2: Density Calculation
- Compute density for each particle:
  ```
  ρᵢ = Σⱼ mⱼ * W_poly6(|rᵢ - rⱼ|, h)
  ```
- Store densities in particle array

### Step 4.3: Pressure Calculation
- Implement Tait equation of state:
  ```
  Pᵢ = B * ((ρᵢ/ρ₀)⁷ - 1)
  ```
  where B = stiffness constant, ρ₀ = rest density
- Handle negative pressures (set to zero)

### Step 4.4: Pressure Force Computation
- Compute pressure force for each particle:
  ```
  F_pressure,i = -Σⱼ mⱼ * (Pᵢ + Pⱼ)/(2*ρⱼ) * ∇W_spiky(|rᵢ - rⱼ|, h)
  ```
- Add to particle accelerations

### Step 4.5: Viscosity Force Computation
- Compute viscosity force:
  ```
  F_viscosity,i = μ * Σⱼ mⱼ * (vⱼ - vᵢ)/ρⱼ * ∇²W_viscosity(|rᵢ - rⱼ|, h)
  ```
  where μ = viscosity coefficient
- Add to particle accelerations

### Step 4.6: External Forces
- Add gravity: F_gravity = m * g
- Apply to particle accelerations

### Step 4.7: Integrate SPH Physics
- Combine all forces into velocity update
- Update positions with Velocity Verlet

### Step 4.8: Basic SPH Simulation Test
- Create dam break scenario
- Verify fluid-like behavior

---

## Phase 5: Advanced SPH Features

### Step 5.1: Surface Tension (Optional)
- Implement surface tension using curvature-based approach
- Or use color field method for droplet formation

### Step 5.2: Adaptive Time Stepping
- Implement CFL condition:
  ```
  dt = CFL * h / max(|v|)
  ```
- Clamp dt to min/max values

### Step 5.3: Density Relaxation
- Add density relaxation step for better volume conservation
- Implement density correction forces

---

## Phase 6: Rendering Enhancements

### Step 6.1: Density-Based Coloring
- Map density values to color gradient (blue to white)
- Pass density to fragment shader via vertex attributes

### Step 6.2: Velocity-Based Coloring
- Alternative visualization: color based on velocity magnitude
- Add toggle between density and velocity coloring modes

### Step 6.3: Particle Size Variation
- Adjust particle size based on pressure
- Low pressure = larger particles (expansion)
- High pressure = smaller particles (compression)

### Step 6.4: Metaball Shader (Optional)
- Implement fragment shader for smooth fluid appearance
- Use marching squares or raymarching approach
- Blend particles using Gaussian kernel

---

## Phase 7: Performance Optimizations

### Step 7.1: GPU Compute Shaders
- Port density calculation to compute shader
- Port force calculations to compute shader
- Use SSBOs for particle data

### Step 7.2: SIMD Vectorization (if CPU-bound)
- Use AVX2 for vectorized particle calculations
- Process 4-8 particles simultaneously

### Step 7.3: Multi-threading
- Implement thread pool for parallel physics
- Partition grid regions among threads
- Use atomic operations or lock-free data structures

### Step 7.4: Profiling
- Add performance counters
- Identify bottlenecks with timing
- Optimize hot paths

---

## Phase 8: User Controls

### Step 8.1: Interactive Particle Spawning
- Detect mouse clicks in OpenGL window
- Spawn particles at mouse position
- Add continuous spawning while mouse held

### Step 8.2: Parameter Tweaking
- Add key bindings for adjusting:
  - Smoothing length (h)
  - Stiffness (B)
  - Viscosity (μ)
  - Gravity (g)
- Display current parameters on screen

### Step 8.3: Performance Metrics Display
- Render FPS counter
- Show particle count
- Display simulation time
- Show neighbor count statistics

---

## File Structure

```
sph2/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── particles.hpp
│   ├── particles.cpp
│   ├── physics.hpp
│   ├── physics.cpp
│   ├── kernels.hpp
│   ├── kernels.cpp
│   ├── spatial.hpp
│   ├── spatial.cpp
│   ├── renderer.hpp
│   ├── renderer.cpp
│   └── utils.hpp
├── shaders/
│   ├── basic.vert
│   ├── basic.frag
│   └── (additional shaders as needed)
└── README.md
```

---

## Key Design Decisions

### Memory Layout
- **Structure of Arrays (SoA)** for cache efficiency
- Contiguous memory allocations
- Pre-allocate capacity to avoid reallocations

### Spatial Partitioning
- **Uniform grid** with cell size = smoothing length
- Fixed-size arrays per cell (avoid dynamic allocation)
- Rebuild grid each frame (acceptable for dynamic simulations)

### Numerical Stability
- **Velocity Verlet** integrator for energy conservation
- **CFL condition** for adaptive time stepping
- **Clamped negative pressures** to prevent cavitation

### Performance Targets
- 10,000+ particles at 60 FPS on modern hardware
- Sub-millisecond neighbor search
- Efficient GPU compute utilization

---

## Testing Strategy

After each phase, verify with:
1. **Unit tests** for mathematical kernels
2. **Visual inspection** of particle behavior
3. **Performance benchmarks** (FPS, particle count)
4. **Stability tests** (energy conservation, particle clumping)
