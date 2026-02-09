#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "particles.hpp"
#include "spatial.hpp"
#include "kernels.hpp"
#include <cmath>
#include <limits>

// Centralized SPH Parameters structure
struct SPHParameters {
    // Spatial parameters
    float h = 0.08f;              // Smoothing length
    float m = 0.02f;              // Particle mass
    
    // Fluid properties
    float rho0 = 550.0f;          // Rest density (kg/m³)
    float B = 50.0f;              // Pressure stiffness
    float gamma = 7.0f;           // Pressure exponent
    float mu = 0.1f;              // Viscosity coefficient
    
    // Simulation parameters
    float dt = 0.016f;            // Time step
    float minDt = 0.0001f;        // Minimum time step
    float maxDt = 0.01f;          // Maximum time step
    float CFL = 0.4f;             // Courant-Friedrichs-Lewy number
    
    // Physics parameters
    float gravity = -9.81f;       // Gravity (m/s²)
    float damping = 0.8f;         // Boundary damping
    
    // Stability parameters
    float maxAcceleration = 50.0f;  // Maximum acceleration magnitude
    float maxVelocity = 100.0f;      // Maximum velocity magnitude (increased from 10.0f)
    
    // Adaptive timestep enable
    bool adaptiveTimestep = true;
};

struct PhysicsParams {
    float dt;
    float gravity;
    float damping;
    float B;          // Pressure stiffness
    float rho0;       // Rest density
    float gamma;      // Pressure exponent
    float mu;         // Viscosity coefficient
};

class Physics {
public:
    Physics(float dt = 0.016f, float gravity = -9.81f, float damping = 0.8f,
             float B = 100.0f, float rho0 = 1000.0f, float gamma = 7.0f, float mu = 0.1f)
        : params{dt, gravity, damping, B, rho0, gamma, mu} {}

    void velocityVerletStep1(Particles& particles);
    void velocityVerletStep2(Particles& particles);
    void handleBoundaries(Particles& particles, float left, float right, float bottom, float top);
    void resetAccelerations(Particles& particles);
    void computePressures(Particles& particles);
    void computePressureForces(Particles& particles, const SpatialHash& grid);
    void computeViscosityForces(Particles& particles, const SpatialHash& grid);
    void applyGravity(Particles& particles);

    // Phase 9: Adaptive time stepping
    float computeAdaptiveTimestep(const Particles& particles, float h);
    
    // Phase 9: Numerical stability checks
    bool checkStability(const Particles& particles);
    bool validateParticleData(const Particles& particles);
    bool checkForNaNOrInf(const Particles& particles);
    
    // Phase 9: Auto-reset on explosion
    void resetSimulationIfUnstable(Particles& particles, int cols, int rows, float spacing, float startX, float startY);

    void setGravity(float g) { params.gravity = g; }
    void setDamping(float d) { params.damping = d; }
    void setTimestep(float dt) { params.dt = dt; }
    void setStiffness(float B) { params.B = B; }
    void setRestDensity(float rho0) { params.rho0 = rho0; }
    void setPressureExponent(float gamma) { params.gamma = gamma; }
    void setViscosity(float mu) { params.mu = mu; }

private:
    PhysicsParams params;
};

#endif
