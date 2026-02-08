#include "physics.hpp"
#include <algorithm>
#include <cmath>
#include "kernels.hpp"

void Physics::resetAccelerations(Particles& particles) {
    size_t n = particles.size();
    for (size_t i = 0; i < n; ++i) {
        particles.accelerations[i] = glm::vec2(0.0f);
    }
}

void Physics::velocityVerletStep1(Particles& particles) {
    size_t n = particles.size();
    
    for (size_t i = 0; i < n; ++i) {
        particles.velocities[i] += 0.5f * particles.accelerations[i] * params.dt;
        particles.positions[i] += particles.velocities[i] * params.dt;
    }
}

void Physics::velocityVerletStep2(Particles& particles) {
    size_t n = particles.size();
    
    for (size_t i = 0; i < n; ++i) {
        particles.velocities[i] += 0.5f * particles.accelerations[i] * params.dt;
    }
}

void Physics::handleBoundaries(Particles& particles, float left, float right, float bottom, float top) {
    size_t n = particles.size();
    
    for (size_t i = 0; i < n; ++i) {
        if (particles.positions[i].x < left) {
            particles.positions[i].x = left;
            particles.velocities[i].x *= -params.damping;
        } else if (particles.positions[i].x > right) {
            particles.positions[i].x = right;
            particles.velocities[i].x *= -params.damping;
        }
        
        if (particles.positions[i].y < bottom) {
            particles.positions[i].y = bottom;
            particles.velocities[i].y *= -params.damping;
        } else if (particles.positions[i].y > top) {
            particles.positions[i].y = top;
            particles.velocities[i].y *= -params.damping;
        }
    }
}

void Physics::computePressures(Particles& particles) {
    size_t n = particles.size();
    
    for (size_t i = 0; i < n; ++i) {
        float ratio = particles.densities[i] / params.rho0;
        particles.pressures[i] = params.B * (pow(ratio, params.gamma) - 1.0f);
        
        // Clamp negative pressures to zero
        if (particles.pressures[i] < 0.0f) {
            particles.pressures[i] = 0.0f;
        }
    }
}

void Physics::applyGravity(Particles& particles) {
    size_t n = particles.size();
    glm::vec2 gravityVec(0.0f, params.gravity);
    
    for (size_t i = 0; i < n; ++i) {
        particles.accelerations[i] += gravityVec;
    }
}

void Physics::computePressureForces(Particles& particles, const SpatialHash& grid) {
    size_t n = particles.size();
    const float h = sph::Kernels::DEFAULT_H;
    const float m = 0.02f; // Particle mass - should match what's used in density calculation
    const float h2 = h * h;
    
    // Use stack buffer for neighbors (no heap allocation)
    size_t neighborBuffer[256];
    
    for (size_t i = 0; i < n; ++i) {
        glm::vec2 f_pressure(0.0f);
        
        // Get neighbors using fast stack-based method
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        // Compute pressure forces from neighbors
        const glm::vec2& pi = particles.positions[i];
        for (size_t k = 0; k < neighborCount; ++k) {
            size_t j = neighborBuffer[k];
            if (i == j) continue;
            
            const glm::vec2& pj = particles.positions[j];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            // Use squared distance check first (avoids sqrt when unnecessary)
            if (r2 < h2 && r2 > 1e-8f) {
                float r = sqrt(r2);
                glm::vec2 r_vec(dx, dy);
                // Use optimized kernel with pre-computed r
                glm::vec2 gradW = sph::Kernels::gradW_spiky(r_vec, r, h);
                float pressure_term = (particles.pressures[i] + particles.pressures[j]) 
                                      / (2.0f * particles.densities[j]);
                f_pressure -= m * pressure_term * gradW;
            }
        }
        
        particles.accelerations[i] += f_pressure / particles.densities[i];
    }
}

void Physics::computeViscosityForces(Particles& particles, const SpatialHash& grid) {
    size_t n = particles.size();
    const float h = sph::Kernels::DEFAULT_H;
    const float m = 0.02f; // Particle mass - should match what's used in density calculation
    const float h2 = h * h;
    const float mu = 0.1f; // Viscosity coefficient
    
    // Use stack buffer for neighbors (no heap allocation)
    size_t neighborBuffer[256];
    
    for (size_t i = 0; i < n; ++i) {
        glm::vec2 f_viscosity(0.0f);
        
        // Get neighbors using fast stack-based method
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        // Compute viscosity forces from neighbors
        const glm::vec2& pi = particles.positions[i];
        const glm::vec2& vi = particles.velocities[i];
        
        for (size_t k = 0; k < neighborCount; ++k) {
            size_t j = neighborBuffer[k];
            if (i == j) continue;
            
            const glm::vec2& pj = particles.positions[j];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            // Use squared distance check first (avoids sqrt when unnecessary)
            if (r2 < h2 && r2 > 1e-8f) {
                float r = sqrt(r2);
                float laplacian = sph::Kernels::laplacianW_viscosity(r, h);
                glm::vec2 velocity_diff = particles.velocities[j] - vi;
                f_viscosity += m * velocity_diff / particles.densities[j] * laplacian;
            }
        }
        
        f_viscosity *= mu;
        particles.accelerations[i] += f_viscosity / particles.densities[i];
        
        // Clamp accelerations to prevent explosion
        const float maxAcceleration = 50.0f;
        float accMagSq = glm::dot(particles.accelerations[i], particles.accelerations[i]);
        if (accMagSq > maxAcceleration * maxAcceleration) {
            float accMag = sqrt(accMagSq);
            particles.accelerations[i] = (particles.accelerations[i] / accMag) * maxAcceleration;
        }
    }
}
