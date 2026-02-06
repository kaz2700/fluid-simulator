#include "sph.hpp"
#include "kernels.hpp"
#include <algorithm>
#include <cmath>

namespace sph {

SPHSolver::SPHSolver(const SPHParams& params) : params(params) {
    neighbors.reserve(100);
}

void SPHSolver::computeDensities(Particles& particles, const SpatialHash& grid) {
    size_t n = particles.size();
    minDensity = std::numeric_limits<float>::max();
    maxDensity = std::numeric_limits<float>::min();
    
    for (size_t i = 0; i < n; ++i) {
        float density = 0.0f;
        
        // Get neighbors using spatial hash
        grid.getNeighbors(i, particles.positions, neighbors);
        
        // Sum contributions from all neighbors
        for (size_t j : neighbors) {
            glm::vec2 diff = particles.positions[i] - particles.positions[j];
            float r = glm::length(diff);
            density += params.m * Kernels::W_poly6(r, params.h);
        }
        
        // Self-contribution
        density += params.m * Kernels::W_poly6(0.0f, params.h);
        
        particles.densities[i] = density;
        
        // Track min/max for visualization
        minDensity = std::min(minDensity, density);
        maxDensity = std::max(maxDensity, density);
    }
}

glm::vec3 densityToColor(float density, float rho0) {
    // Normalize density around rest density
    // Blue = low density, Red = high density
    float t = (density - rho0 * 0.8f) / (rho0 * 0.4f);
    t = std::max(0.0f, std::min(1.0f, t));
    
    // Interpolate between blue (low) and red (high)
    glm::vec3 lowColor(0.0f, 0.3f, 1.0f);   // Blue
    glm::vec3 highColor(1.0f, 0.3f, 0.0f);  // Red
    
    return lowColor * (1.0f - t) + highColor * t;
}

} // namespace sph
