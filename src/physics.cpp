#include "physics.hpp"
#include <algorithm>
#include <cmath>

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
