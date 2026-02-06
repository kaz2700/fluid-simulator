#include "sph.hpp"
#include "kernels.hpp"
#include <algorithm>
#include <cmath>

namespace sph {

SPHSolver::SPHSolver(const SPHParams& params) : params(params) {
}

void SPHSolver::computeDensities(Particles& particles, const SpatialHash& grid) {
    size_t n = particles.size();
    minDensity = std::numeric_limits<float>::max();
    maxDensity = std::numeric_limits<float>::min();
    
    // Pre-allocate neighbor buffer on stack (avoid repeated heap allocations)
    size_t neighborBuffer[256];
    
    for (size_t i = 0; i < n; ++i) {
        float density = 0.0f;
        
        // Get neighbors - pass pre-allocated buffer
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        // Sum contributions from all neighbors
        const glm::vec2& pi = particles.positions[i];
        for (size_t k = 0; k < neighborCount; ++k) {
            const glm::vec2& pj = particles.positions[neighborBuffer[k]];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r = std::sqrt(dx * dx + dy * dy);
            density += params.m * Kernels::W_poly6(r, params.h);
        }
        
        // Self-contribution (W_poly6(0, h) = 315/(64*π*h^9) * h^6 = 315/(64*π*h^3))
        static const float selfContribution = params.m * 315.0f / (64.0f * M_PI * params.h * params.h * params.h);
        density += selfContribution;
        
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
