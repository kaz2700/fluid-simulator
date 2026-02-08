# SPH 2D Fluid Simulator

A high-performance 2D Smoothed Particle Hydrodynamics (SPH) fluid simulation system built with C++ and OpenGL.

## Overview

This simulator implements a Lagrangian particle-based method for simulating fluid dynamics, where fluid is represented by discrete particles that carry physical properties. The system is optimized for performance and handles 10,000+ particles efficiently on modern hardware.

## Features

### ✅ Implemented (Phase 1-9)

- **Basic Rendering**: OpenGL instanced rendering with particle visualization
- **Particle Dynamics**: Velocity Verlet integration with boundary handling
- **Spatial Partitioning**: Uniform grid for O(n) neighbor search
- **SPH Kernels**: Poly6, Spiky gradient, and Viscosity kernels
- **Density Calculation**: Neighbor-based density computation
- **Pressure Forces**: Tait equation of state with pressure forces
- **Viscosity**: Velocity-based viscous forces
- **Gravity**: External gravity forces
- **Adaptive Timestepping**: CFL-based automatic timestep adjustment
- **Stability Checks**: NaN/Inf detection and automatic reset
- **Performance Monitoring**: Real-time FPS and timing breakdown

### 🎯 Performance

- **1,000 particles**: ~60 FPS
- **5,000 particles**: ~30-40 FPS  
- **10,000 particles**: ~15-20 FPS

## Controls

- **D** - Toggle density-based coloring
- **P** - Toggle pressure-based coloring
- **R** - Reset simulation

## Building

### Prerequisites

- C++17 compatible compiler
- CMake 3.10+
- OpenGL 2.1+ 
- GLFW, GLAD, GLM

### Build Steps

```bash
# Clone repository
git clone <repository-url>
cd fluid-simulator

# Build
mkdir build && cd build
cmake ..
make -j4

# Run tests
./test_kernels

# Run simulator
./sph2_sim
```

## Architecture

### Core Components

- **`particles.hpp/cpp`**: Particle data structure (Structure of Arrays layout)
- **`physics.hpp/cpp`**: SPH physics calculations and integration
- **`spatial.hpp/cpp`**: Uniform grid spatial partitioning
- **`kernels.hpp/cpp`**: SPH kernel functions
- **`renderer.hpp/cpp`**: OpenGL rendering system
- **`performance_monitor.hpp/cpp`**: Real-time performance metrics

### Memory Layout

The system uses **Structure of Arrays (SoA)** for optimal cache performance:

```cpp
struct Particles {
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;
    std::vector<glm::vec2> accelerations;
    std::vector<float> densities;
    std::vector<float> pressures;
};
```

### SPH Parameters

Key tunable parameters for simulation stability and behavior:

- **Smoothing length (h)**: Interaction radius (~0.08m)
- **Particle mass (m)**: Mass per particle (0.02kg)
- **Rest density (ρ₀)**: Target density (550kg/m³)
- **Stiffness (B)**: Pressure constant (50.0Pa)
- **Viscosity (μ)**: Fluid thickness (0.1Pa·s)

## Development Status

**Current Phase**: Phase 9 Complete - Stability and Tuning

**Next Phase**: Phase 10 - Rendering Enhancements

The simulator is built incrementally - each phase adds testable functionality. See [PLAN.md](PLAN.md) for detailed development roadmap.

## Testing

```bash
# Run kernel unit tests
cd build && ./test_kernels

# Expected output: 12/12 tests passed
```

## Physics Details

### SPH Equations

The simulator implements physically correct SPH equations:

1. **Density**: ρᵢ = Σⱼ mⱼ W(|rᵢ - rⱼ|, h)
2. **Pressure**: pᵢ = B(ρᵢ/ρ₀)^γ - 1
3. **Pressure Force**: fᵢ^pressure = -Σⱼ mⱼ (pᵢ + pⱼ)/(2ρⱼ) ∇W
4. **Viscosity Force**: fᵢ^viscosity = μ Σⱼ mⱼ (vⱼ - vᵢ)/ρⱼ ∇²W

### Numerical Methods

- **Integrator**: Velocity Verlet for energy conservation
- **Spatial Hash**: Uniform grid with cell size = smoothing length
- **Adaptive Timestep**: CFL condition: dt = 0.4 * h / v_max

## Performance Optimizations

- Structure of Arrays (SoA) memory layout
- Uniform grid spatial partitioning
- Instanced OpenGL rendering
- Stack-based neighbor buffers
- Pre-allocated memory pools
- SIMD-friendly access patterns

## Contributing

This is an academic/educational SPH implementation. For contributions:

1. Follow the incremental development approach in PLAN.md
2. Maintain performance optimization focus
3. Add appropriate unit tests
4. Update documentation

## License

[Add your license here]

## Acknowledgments

Based on standard SPH formulations and performance optimization techniques from computational fluid dynamics research.