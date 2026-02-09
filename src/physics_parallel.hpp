#ifndef PHYSICS_PARALLEL_HPP
#define PHYSICS_PARALLEL_HPP

#include "physics.hpp"
#include "thread_pool.hpp"
#include "sph.hpp"

// Phase 13: Multi-threaded physics computations
class ParallelPhysics {
public:
    ParallelPhysics(ThreadPool& pool, Physics& physics, sph::SPHSolver& solver)
        : threadPool(pool), physics(physics), sphSolver(solver) {}
    
    // Parallel versions of physics computations
    void computeDensitiesParallel(Particles& particles, const SpatialHash& grid, float h, float m);
    void computePressuresParallel(Particles& particles, float rho0, float B, float gamma);
    void computePressureForcesParallel(Particles& particles, const SpatialHash& grid, float h, float m);
    void computeViscosityForcesParallel(Particles& particles, const SpatialHash& grid, float h, float m, float mu);
    void resetAccelerationsParallel(Particles& particles);
    void applyGravityParallel(Particles& particles, float gravity);
    void velocityVerletStep1Parallel(Particles& particles, float dt);
    void velocityVerletStep2Parallel(Particles& particles, float dt);
    void handleBoundariesParallel(Particles& particles, float left, float right, float bottom, float top, float damping);
    
    // Get thread pool size
    size_t getNumThreads() const { return threadPool.size(); }
    
    // Toggle parallel execution
    void setParallelEnabled(bool enabled) { parallelEnabled = enabled; }
    bool isParallelEnabled() const { return parallelEnabled; }

private:
    ThreadPool& threadPool;
    Physics& physics;
    sph::SPHSolver& sphSolver;
    bool parallelEnabled = true;
};

#endif // PHYSICS_PARALLEL_HPP
