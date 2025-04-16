#ifndef particle_header
#define particle_header

struct {
    float position[2];
    float velocity[2];
    float acceleration[2];
    float radius;
    float mass;
    float charge;
} typedef Particle;

#endif // ADDITIONAL_H