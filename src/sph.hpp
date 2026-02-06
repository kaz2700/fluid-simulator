#ifndef SPH_HPP
#define SPH_HPP

#include "particles.hpp"
#include "spatial.hpp"
#include <vector>

namespace sph {

struct SPHParams {
    float h;           // Smoothing length
    float m;           // Particle mass
    float rho0;        // Rest density
    float B;           // Stiffness
    float mu;          // Viscosity
    
    SPHParams(float h = 0.1f, float m = 0.02f, float rho0 = 1000.0f, 
              float B = 200.0f, float mu = 0.1f)
        : h(h), m(m), rho0(rho0), B(B), mu(mu) {}
};

class SPHSolver {
public:
    SPHSolver(const SPHParams& params = SPHParams());
    
    // Compute densities using Poly6 kernel
    void computeDensities(Particles& particles, const SpatialHash& grid);
    
    // Get min/max density for visualization normalization
    float getMinDensity() const { return minDensity; }
    float getMaxDensity() const { return maxDensity; }
    float getRestDensity() const { return params.rho0; }
    
private:
    SPHParams params;
    float minDensity;
    float maxDensity;
};

// Convert density to color (blue = low, red = high)
glm::vec3 densityToColor(float density, float rho0);

} // namespace sph

#endif // SPH_HPP
