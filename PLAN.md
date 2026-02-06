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
**Phase**: Phase 4 Complete - Kernel Functions Implemented  
**Next**: Phase 5 - Density Calculation

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

## Phase 1: Project Setup & Basic Rendering ✅ COMPLETED

### Step 1.1: Initialize CMake Project ✅
- Create CMakeLists.txt with minimum C++17 standard
- Set up GLFW and GLAD dependencies
- Configure build system for target platform

### Step 1.2: OpenGL Context Creation ✅
- Initialize GLFW window
- Create OpenGL context (3.3+ core profile)
- Set up GLAD loader
- Configure viewport and basic OpenGL state

### Step 1.3: Particle Data Structure (SoA) ✅
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

### Step 1.4: Basic Shaders ✅
- Create vertex shader for particle rendering
- Create fragment shader for particle coloring
- Compile and link shader program

### Step 1.5: Particle Rendering System ✅
- Create vertex array object (VAO) and vertex buffer objects (VBOs)
- Implement instanced rendering for performance
- Render static particles as test

### Step 1.6: Main Loop Structure ✅
- Set up game loop with delta time
- Implement clear and swap buffers
- Add FPS counter

---

## Phase 2: Particle Movement ✅ COMPLETED

### Step 2.1: Time Integration System ✅
- Implement Velocity Verlet integrator:
  ```cpp
  v(t+0.5*dt) = v(t) + 0.5 * a(t) * dt
  x(t+dt) = x(t) + v(t+0.5*dt) * dt
  // Compute new accelerations
  v(t+dt) = v(t+0.5*dt) + 0.5 * a(t+dt) * dt
  ```

### Step 2.2: Initial Particle Configuration ✅
- Create function to spawn particles in grid pattern
- Set initial velocities (can be zero)

### Step 2.3: Boundary Handling ✅
- Implement reflective boundary conditions:
  - Detect collision with domain edges
  - Reflect velocity component perpendicular to wall
  - Apply damping factor for energy loss

### Step 2.4: Basic Motion Test ✅
- Give particles random initial velocities
- Verify they move correctly and bounce off walls

---

## Phase 3: Spatial Partitioning (Performance Critical) ✅ COMPLETED

### Step 3.1: Uniform Grid Structure ✅
- Design grid cell size based on smoothing length h
- Implement spatial hash function:
  ```cpp
  int cellX = floor((x - originX) / cellSize);
  int cellY = floor((y - originY) / cellSize);
  ```

### Step 3.2: Particle-Cell Mapping ✅
- Create 2D grid array storing particle indices
- Implement update function to rebuild grid each frame
- Consider using fixed-size arrays per cell to avoid allocations

### Step 3.3: Neighbor Search ✅
- Implement function to find neighbors within smoothing length h:
  - Query grid cells within ±1 cell radius
- Return list of neighbor indices

### Step 3.4: Performance Testing ✅
- Benchmark neighbor search with 1000, 5000, 10000 particles
- Compare O(n²) vs O(n) neighbor search performance
- **Results:**
  - 100 particles: 0.06ms, ~1 neighbor avg
  - 2,500 particles: 5ms, ~15 neighbors avg
  - 10,000 particles: 36ms, ~65 neighbors avg (at start) → ~54 neighbors avg (after spreading)

---

## Phase 4: Kernel Functions (Mathematical Foundation) ✅ COMPLETED

### Step 4.1: Create Kernels Module ✅
- Create `kernels.hpp` and `kernels.cpp` files
- Define smoothing length `h` as a constant parameter

### Step 4.2: Implement Poly6 Kernel ✅
```cpp
float W_poly6(float r, float h) {
    if (r > h) return 0.0f;
    float term = h*h - r*r;
    return (315.0f / (64.0f * M_PI * pow(h, 9))) * pow(term, 3);
}
```
- **Test**: Verify kernel returns 0 when r > h
- **Test**: Verify kernel is maximum at r = 0
- **Test**: Verify kernel integrates to ~1 over its support radius

### Step 4.3: Implement Spiky Kernel Gradient ✅
```cpp
glm::vec2 gradW_spiky(const glm::vec2& r_vec, float h) {
    float r = glm::length(r_vec);
    if (r > h || r < 1e-6f) return glm::vec2(0.0f);
    float coeff = -45.0f / (M_PI * pow(h, 6)) * pow(h - r, 2) / r;
    return coeff * r_vec;
}
```
- **Test**: Verify gradient points away from center
- **Test**: Verify magnitude decreases with distance

### Step 4.4: Implement Viscosity Kernel Laplacian ✅
```cpp
float laplacianW_viscosity(float r, float h) {
    if (r > h) return 0.0f;
    return 45.0f / (M_PI * pow(h, 6)) * (h - r);
}
```
- **Test**: Verify always non-negative
- **Test**: Verify zero at r = h

### Step 4.5: Unit Tests ✅
- Test kernel values at boundary (r = h)
- Test kernel values at center (r = 0)
- Verify mathematical properties (symmetry, smoothness)
- **Results**: 12/12 tests passed

---

## Phase 5: Density Calculation

### Step 5.1: Add Density to Particle Structure
- Add `std::vector<float> densities` to Particles struct
- Initialize all densities to rest density (ρ₀)

### Step 5.2: Compute Particle Densities
```cpp
void computeDensities(Particles& particles, const SpatialGrid& grid) {
    for each particle i:
        ρᵢ = 0
        for each neighbor j:
            r = |xᵢ - xⱼ|
            ρᵢ += m * W_poly6(r, h)
        // Self-contribution
        ρᵢ += m * W_poly6(0, h)
}
```
- Use existing spatial grid for neighbor search
- Include self-contribution term

### Step 5.3: Density Visualization
- Pass density values to shader as vertex attribute
- Color particles based on density (blue = low, red = high)
- **Visual Test**: Particles should show varying colors

### Step 5.4: Validation
- **Test**: Uniform grid of particles should have roughly uniform density
- **Test**: Particles at edges may have lower density (expected)
- **Test**: Verify density values are reasonable (not zero, not NaN)

---

## Phase 6: Pressure Calculation

### Step 6.1: Add Pressure to Particle Structure
- Add `std::vector<float> pressures` to Particles struct

### Step 6.2: Implement Tait Equation of State
```cpp
void computePressures(Particles& particles) {
    const float B = 100.0f;        // Stiffness (tune this)
    const float rho0 = 1000.0f;    // Rest density (kg/m³)
    const float gamma = 7.0f;      // Exponent
    
    for each particle i:
        float ratio = particles.densities[i] / rho0;
        particles.pressures[i] = B * (pow(ratio, gamma) - 1.0f);
        // Clamp negative pressures to zero
        if (particles.pressures[i] < 0.0f) 
            particles.pressures[i] = 0.0f;
}
```

### Step 6.3: Visualize Pressure
- Color particles based on pressure (green = low, yellow = high)
- **Visual Test**: Higher density areas should show higher pressure

### Step 6.4: Parameter Tuning
- Start with B = 100.0f (low stiffness, more compressible)
- Test with B = 1000.0f (higher stiffness, less compressible)
- Verify pressure values are positive and finite

---

## Phase 7: Pressure Forces

### Step 7.1: Add Pressure Acceleration Calculation
```cpp
void computePressureForces(Particles& particles, const SpatialGrid& grid) {
    for each particle i:
        glm::vec2 f_pressure(0.0f);
        for each neighbor j:
            if (i == j) continue;
            glm::vec2 r_vec = particles.positions[i] - particles.positions[j];
            float r = glm::length(r_vec);
            if (r < h) {
                glm::vec2 gradW = gradW_spiky(r_vec, h);
                float pressure_term = (particles.pressures[i] + particles.pressures[j]) 
                                      / (2.0f * particles.densities[j]);
                f_pressure -= m * pressure_term * gradW;
            }
        }
        particles.accelerations[i] = f_pressure / particles.densities[i];
}
```

### Step 7.2: Integrate with Velocity Verlet
- Use pressure forces in the integration step
- Keep boundary handling from Phase 2

### Step 7.3: First Fluid Motion Test
- Initialize particles in a square block
- Run simulation with pressure forces only
- **Visual Test**: Particles should repel each other and expand
- **Visual Test**: Fluid should push away from high-pressure regions

### Step 7.4: Debug and Validate
- Check for NaN or infinite values
- Verify particles don't explode (too much force)
- Tune B parameter if needed (reduce if unstable)

---

## Phase 8: Viscosity and External Forces

### Step 8.1: Add Viscosity Forces
```cpp
void computeViscosityForces(Particles& particles, const SpatialGrid& grid) {
    const float mu = 0.1f;  // Viscosity coefficient (tune this)
    
    for each particle i:
        glm::vec2 f_viscosity(0.0f);
        for each neighbor j:
            if (i == j) continue;
            float r = distance(particles.positions[i], particles.positions[j]);
            if (r < h) {
                float laplacian = laplacianW_viscosity(r, h);
                glm::vec2 velocity_diff = particles.velocities[j] - particles.velocities[i];
                f_viscosity += m * velocity_diff / particles.densities[j] * laplacian;
            }
        }
        f_viscosity *= mu;
        particles.accelerations[i] += f_viscosity / particles.densities[i];
}
```

### Step 8.2: Add Gravity
```cpp
void applyGravity(Particles& particles) {
    const glm::vec2 gravity(0.0f, -9.81f);  // m/s² downward
    for each particle i:
        particles.accelerations[i] += gravity;
}
```

### Step 8.3: Complete Force Integration
Update the main loop:
```cpp
1. Compute densities
2. Compute pressures
3. Reset accelerations to zero
4. Compute pressure forces (adds to accelerations)
5. Compute viscosity forces (adds to accelerations)
6. Apply gravity (adds to accelerations)
7. Velocity Verlet integration
8. Boundary handling
```

### Step 8.4: Full SPH Test
- **Dam Break Test**: Initialize fluid block, remove barrier, verify flow
- **Visual Test**: Fluid should flow downward due to gravity
- **Visual Test**: Viscosity should dampen motion (compare with/without)
- **Visual Test**: Fluid should be incompressible-ish (maintain volume)

---

## Phase 9: Stability and Tuning

### Step 9.1: Adaptive Time Stepping
```cpp
float computeAdaptiveTimestep(const Particles& particles) {
    const float CFL = 0.4f;  // Courant-Friedrichs-Lewy number
    float max_velocity = 0.0f;
    for each particle i:
        max_velocity = max(max_velocity, glm::length(particles.velocities[i]));
    if (max_velocity < 1e-6f) return MAX_DT;
    float dt = CFL * h / max_velocity;
    return glm::clamp(dt, MIN_DT, MAX_DT);  // e.g., 0.0001f to 0.01f
}
```
- Prevents instability when velocities get high
- Essential for maintaining simulation stability

### Step 9.2: Parameter Space Exploration
Create a parameter file for easy tuning:
```cpp
struct SPHParams {
    float h = 0.1f;           // Smoothing length
    float m = 0.02f;          // Particle mass
    float rho0 = 1000.0f;     // Rest density
    float B = 200.0f;         // Stiffness
    float mu = 0.1f;          // Viscosity
    float damping = 0.8f;     // Boundary damping
};
```
- Test different values systematically
- Document good parameter combinations

### Step 9.3: Numerical Stability Checks
- Add assertions for NaN/Inf detection
- Check for negative densities (shouldn't happen)
- Verify particles stay within reasonable bounds
- Add automatic simulation reset if explosion detected

### Step 9.4: Performance Baseline
- Measure FPS with 1000, 5000, 10000 particles
- Record timing for each physics step
- Establish baseline before optimizations

---

## Phase 10: Rendering Enhancements

### Step 10.1: Density-Based Coloring
```cpp
// In fragment shader
vec3 densityToColor(float density) {
    float t = (density - rho0 * 0.8f) / (rho0 * 0.4f);
    t = clamp(t, 0.0f, 1.0f);
    return mix(vec3(0.0, 0.3, 1.0), vec3(1.0, 0.3, 0.0), t);
}
```
- Blue = low density, Red = high density
- Pass density as vertex attribute

### Step 10.2: Velocity-Based Coloring
```cpp
vec3 velocityToColor(vec2 velocity) {
    float speed = length(velocity);
    float t = clamp(speed / maxSpeed, 0.0f, 1.0f);
    return mix(vec3(0.0, 0.0, 0.5), vec3(1.0, 1.0, 0.0), t);
}
```
- Toggle between density and velocity coloring with key press

### Step 10.3: Particle Size Based on Pressure
```cpp
float size = baseSize * (1.0f + 0.2f * (pressure / B));
```
- Higher pressure = larger particles (helps visualize compression)

### Step 10.4: Simple Background Grid
- Render grid lines behind particles
- Helps visualize fluid motion and boundaries

---

## Phase 11: User Interaction

### Step 11.1: Mouse Controls
- Left click + drag: Add particles continuously
- Right click: Remove particles near cursor
- Scroll wheel: Adjust zoom level

### Step 11.2: Keyboard Controls
```cpp
Keys:
  R - Reset simulation
  Space - Pause/Resume
  1 - Density coloring
  2 - Velocity coloring
  3 - Pressure coloring
  Up/Down - Increase/decrease gravity
  Left/Right - Increase/decrease viscosity
```

### Step 11.3: On-Screen Display (OSD)
- Render text showing:
  - FPS
  - Particle count
  - Current parameters (gravity, viscosity, stiffness)
  - Simulation time
- Use simple bitmap font or OpenGL text rendering

### Step 11.4: Scenario Presets
Create preset initial conditions:
- Dam break (block of water)
- Water drop (circular cluster falling)
- Double dam break (two fluids colliding)
- Fountain (continuous particle source)

---

## Phase 12: Performance Optimizations (CPU)

### Step 12.1: Code Profiling Infrastructure
```cpp
class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    void begin() { start = std::chrono::high_resolution_clock::now(); }
    float elapsed() { 
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float, std::milli>(end - start).count();
    }
};
```
- Time each major step: neighbor search, density, pressure, integration
- Print timing breakdown each frame

### Step 12.2: Memory Optimization
- Use `reserve()` for all vectors to prevent reallocations
- Consider using `std::vector` pools for temporary arrays
- Profile memory usage

### Step 12.3: Cache-Friendly Access Patterns
- Ensure spatial grid traversal accesses memory sequentially
- Minimize pointer chasing in neighbor loops
- Consider reordering particles by spatial locality (optional)

### Step 12.4: Spatial Grid Optimizations
- Use bitsets or boolean flags instead of clearing entire grid
- Consider using z-order curve for better cache locality (advanced)
- Experiment with different cell sizes (h, 2h, 0.5h)

---

## Phase 13: Multi-threading (Optional Performance Boost)

### Step 13.1: Thread Pool Implementation
- Create thread pool with hardware concurrency threads
- Use work-stealing queue or simple task distribution

### Step 13.2: Parallelize Physics Steps
```cpp
// Parallel for each particle
#pragma omp parallel for  // or use thread pool
for (int i = 0; i < particleCount; ++i) {
    computeDensityForParticle(i);
}
```
- Density calculation: Embarrassingly parallel
- Pressure/viscosity: Requires careful synchronization or domain decomposition

### Step 13.3: Thread Safety
- Use atomic operations for grid updates (if parallelizing grid build)
- Or partition grid into regions processed by different threads
- Ensure no race conditions in force calculations

### Step 13.4: Performance Validation
- Compare single-threaded vs multi-threaded performance
- Verify correctness (same results as single-threaded)
- Measure scaling (should see ~2x speedup on 4 cores for density calc)

---

## Phase 14: GPU Compute (Advanced)

### Step 14.1: OpenGL Compute Shader Setup
- Create compute shader for density calculation
- Use Shader Storage Buffer Objects (SSBOs) for particle data
- Understand compute shader work groups

### Step 14.2: GPU Density Calculation
- Port density kernel to compute shader
- Use parallel reduction for neighbor search (or spatial hash on GPU)
- Read results back to CPU or keep on GPU

### Step 14.3: GPU Force Calculation
- Port pressure and viscosity calculations to compute shader
- Benchmark CPU vs GPU performance
- Consider hybrid approach (CPU for small particle counts, GPU for large)

### Step 14.4: Zero-Copy Rendering
- Use SSBOs directly in vertex shader
- Eliminate CPU→GPU data transfer each frame
- Requires keeping all particle data on GPU

---

## Phase 15: Advanced Features (Optional)

### Step 15.1: Surface Tension
- Implement surface tension using curvature-based approach
- Or use color field method for droplet formation
- Helps create realistic droplets and splashes

### Step 15.2: Density Relaxation / Incompressibility
- Add density relaxation step for better volume conservation
- Implement Predictive-Corrective Incompressible SPH (PCISPH)
- Or implement divergence-free SPH (DFSPH) for true incompressibility

### Step 15.3: Rigid Body Coupling
- Add rigid bodies that interact with fluid
- Compute fluid-solid forces
- Allow objects to float or sink based on density

### Step 15.4: Multiple Fluid Types
- Support different fluids with varying properties
- Handle fluid-fluid interaction (e.g., oil and water)
- Different colors for different fluid types

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
