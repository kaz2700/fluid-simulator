#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "particles.hpp"

struct PhysicsParams {
    float dt;
    float gravity;
    float damping;
};

class Physics {
public:
    Physics(float dt = 0.016f, float gravity = -9.81f, float damping = 0.8f)
        : params{dt, gravity, damping} {}

    void velocityVerletStep1(Particles& particles);
    void velocityVerletStep2(Particles& particles);
    void handleBoundaries(Particles& particles, float left, float right, float bottom, float top);

    void setGravity(float g) { params.gravity = g; }
    void setDamping(float d) { params.damping = d; }
    void setTimestep(float dt) { params.dt = dt; }

private:
    PhysicsParams params;
};

#endif
