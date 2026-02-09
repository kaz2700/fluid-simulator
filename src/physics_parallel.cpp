#include "physics_parallel.hpp"
#include "kernels.hpp"
#include <cmath>

// Phase 13: Parallel density computation
void ParallelPhysics::computeDensitiesParallel(Particles& particles, const SpatialHash& grid, float h, float m) {
    if (!parallelEnabled) {
        sphSolver.computeDensities(particles, grid);
        return;
    }
    
    size_t n = particles.size();
    const float h2 = h * h;
    const float h6 = h2 * h2 * h2;
    const float h9 = h6 * h2 * h;
    const float poly6Coeff = 315.0f / (64.0f * M_PI * h9);
    const float selfContribution = m * poly6Coeff * h2 * h2 * h2;
    
    threadPool.parallelFor(0, n, [&](size_t i) {
        float density = 0.0f;
        
        // Stack buffer for neighbors
        size_t neighborBuffer[256];
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        // Sum contributions from neighbors
        const glm::vec2& pi = particles.positions[i];
        for (size_t k = 0; k < neighborCount; ++k) {
            const glm::vec2& pj = particles.positions[neighborBuffer[k]];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            if (r2 < h2) {
                float diff = h2 - r2;
                float diff3 = diff * diff * diff;
                density += m * poly6Coeff * diff3;
            }
        }
        
        // Self-contribution
        density += selfContribution;
        particles.densities[i] = density;
    });
}

// Phase 13: Parallel pressure computation
void ParallelPhysics::computePressuresParallel(Particles& particles, float rho0, float B, float gamma) {
    if (!parallelEnabled) {
        // Fall back to sequential for small particle counts
        size_t n = particles.size();
        for (size_t i = 0; i < n; ++i) {
            float ratio = particles.densities[i] / rho0;
            particles.pressures[i] = B * (pow(ratio, gamma) - 1.0f);
            if (particles.pressures[i] < 0.0f) {
                particles.pressures[i] = 0.0f;
            }
        }
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        float ratio = particles.densities[i] / rho0;
        float pressure = B * (pow(ratio, gamma) - 1.0f);
        // Clamp negative pressures
        particles.pressures[i] = (pressure < 0.0f) ? 0.0f : pressure;
    });
}

// Phase 13: Parallel pressure forces computation
void ParallelPhysics::computePressureForcesParallel(Particles& particles, const SpatialHash& grid, float h, float m) {
    if (!parallelEnabled) {
        physics.computePressureForces(particles, grid);
        return;
    }
    
    size_t n = particles.size();
    const float h2 = h * h;
    
    threadPool.parallelFor(0, n, [&](size_t i) {
        glm::vec2 f_pressure(0.0f);
        
        // Stack buffer for neighbors
        size_t neighborBuffer[256];
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        const glm::vec2& pi = particles.positions[i];
        for (size_t k = 0; k < neighborCount; ++k) {
            size_t j = neighborBuffer[k];
            if (i == j) continue;
            
            const glm::vec2& pj = particles.positions[j];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            if (r2 < h2 && r2 > 1e-8f) {
                float r = sqrt(r2);
                glm::vec2 r_vec(dx, dy);
                glm::vec2 gradW = sph::Kernels::gradW_spiky(r_vec, r, h);
                float pressure_term = (particles.pressures[i] + particles.pressures[j]) 
                                      / (2.0f * particles.densities[j]);
                f_pressure -= m * pressure_term * gradW;
            }
        }
        
        particles.accelerations[i] += f_pressure / particles.densities[i];
    });
}

// Phase 13: Parallel viscosity forces computation
void ParallelPhysics::computeViscosityForcesParallel(Particles& particles, const SpatialHash& grid, float h, float m, float mu) {
    if (!parallelEnabled) {
        physics.computeViscosityForces(particles, grid);
        return;
    }
    
    size_t n = particles.size();
    const float h2 = h * h;
    const float maxAcceleration = 50.0f;
    
    threadPool.parallelFor(0, n, [&](size_t i) {
        glm::vec2 f_viscosity(0.0f);
        
        // Stack buffer for neighbors
        size_t neighborBuffer[256];
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        const glm::vec2& pi = particles.positions[i];
        const glm::vec2& vi = particles.velocities[i];
        
        for (size_t k = 0; k < neighborCount; ++k) {
            size_t j = neighborBuffer[k];
            if (i == j) continue;
            
            const glm::vec2& pj = particles.positions[j];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            if (r2 < h2 && r2 > 1e-8f) {
                float r = sqrt(r2);
                float laplacian = sph::Kernels::laplacianW_viscosity(r, h);
                glm::vec2 velocity_diff = particles.velocities[j] - vi;
                f_viscosity += m * velocity_diff / particles.densities[j] * laplacian;
            }
        }
        
        f_viscosity *= mu;
        particles.accelerations[i] += f_viscosity / particles.densities[i];
        
        // Clamp accelerations
        float accMagSq = glm::dot(particles.accelerations[i], particles.accelerations[i]);
        if (accMagSq > maxAcceleration * maxAcceleration) {
            float accMag = sqrt(accMagSq);
            particles.accelerations[i] = (particles.accelerations[i] / accMag) * maxAcceleration;
        }
    });
}

// Phase 13: Parallel reset accelerations
void ParallelPhysics::resetAccelerationsParallel(Particles& particles) {
    if (!parallelEnabled) {
        physics.resetAccelerations(particles);
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        particles.accelerations[i] = glm::vec2(0.0f);
    });
}

// Phase 13: Parallel gravity application
void ParallelPhysics::applyGravityParallel(Particles& particles, float gravity) {
    if (!parallelEnabled) {
        // Fall back to sequential
        size_t n = particles.size();
        glm::vec2 gravityVec(0.0f, gravity);
        for (size_t i = 0; i < n; ++i) {
            particles.accelerations[i] += gravityVec;
        }
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        particles.accelerations[i].y += gravity;
    });
}

// Phase 13: Parallel Velocity Verlet step 1
void ParallelPhysics::velocityVerletStep1Parallel(Particles& particles, float dt) {
    if (!parallelEnabled) {
        // Fall back to sequential
        size_t n = particles.size();
        for (size_t i = 0; i < n; ++i) {
            particles.velocities[i] += 0.5f * particles.accelerations[i] * dt;
            particles.positions[i] += particles.velocities[i] * dt;
        }
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        particles.velocities[i] += 0.5f * particles.accelerations[i] * dt;
        particles.positions[i] += particles.velocities[i] * dt;
    });
}

// Phase 13: Parallel Velocity Verlet step 2
void ParallelPhysics::velocityVerletStep2Parallel(Particles& particles, float dt) {
    if (!parallelEnabled) {
        // Fall back to sequential
        size_t n = particles.size();
        for (size_t i = 0; i < n; ++i) {
            particles.velocities[i] += 0.5f * particles.accelerations[i] * dt;
        }
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        particles.velocities[i] += 0.5f * particles.accelerations[i] * dt;
    });
}

// Phase 13: Parallel boundary handling
void ParallelPhysics::handleBoundariesParallel(Particles& particles, float left, float right, float bottom, float top, float damping) {
    if (!parallelEnabled) {
        physics.handleBoundaries(particles, left, right, bottom, top);
        return;
    }
    
    size_t n = particles.size();
    threadPool.parallelFor(0, n, [&](size_t i) {
        if (particles.positions[i].x < left) {
            particles.positions[i].x = left;
            particles.velocities[i].x *= -damping;
        } else if (particles.positions[i].x > right) {
            particles.positions[i].x = right;
            particles.velocities[i].x *= -damping;
        }
        
        if (particles.positions[i].y < bottom) {
            particles.positions[i].y = bottom;
            particles.velocities[i].y *= -damping;
        } else if (particles.positions[i].y > top) {
            particles.positions[i].y = top;
            particles.velocities[i].y *= -damping;
        }
    });
}
