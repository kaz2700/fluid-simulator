#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "particles.hpp"
#include "spatial.hpp"
#include "kernels.hpp"

struct PhysicsParams {
    float dt;
    float gravity;
    float damping;
    float B;          // Pressure stiffness
    float rho0;       // Rest density
    float gamma;      // Pressure exponent
};

class Physics {
public:
    Physics(float dt = 0.016f, float gravity = -9.81f, float damping = 0.8f,
             float B = 100.0f, float rho0 = 1000.0f, float gamma = 7.0f)
        : params{dt, gravity, damping, B, rho0, gamma} {}

    void velocityVerletStep1(Particles& particles);
    void velocityVerletStep2(Particles& particles);
    void handleBoundaries(Particles& particles, float left, float right, float bottom, float top);
    void resetAccelerations(Particles& particles);
    void computePressures(Particles& particles);
    void computePressureForces(Particles& particles, const SpatialHash& grid);
    void computeViscosityForces(Particles& particles, const SpatialHash& grid);
    void applyGravity(Particles& particles);

    void setGravity(float g) { params.gravity = g; }
    void setDamping(float d) { params.damping = d; }
    void setTimestep(float dt) { params.dt = dt; }
    void setStiffness(float B) { params.B = B; }
    void setRestDensity(float rho0) { params.rho0 = rho0; }
    void setPressureExponent(float gamma) { params.gamma = gamma; }

private:
    PhysicsParams params;
};

#endif
