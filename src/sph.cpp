#include "sph.hpp"
#include "kernels.hpp"
#include <cmath>

namespace sph {

SPHSolver::SPHSolver(const SPHParams& params) : params(params) {
}

void SPHSolver::computeDensities(Particles& particles, const SpatialHash& grid) {
    size_t n = particles.size();
    
    // Pre-compute kernel constant: 315/(64*π*h⁹)
    const float h2 = params.h * params.h;
    const float h6 = h2 * h2 * h2;
    const float h9 = h6 * h2 * params.h;
    const float poly6Coeff = 315.0f / (64.0f * M_PI * h9);
    const float selfContribution = params.m * poly6Coeff * h2 * h2 * h2; // h^6
    
    // Pre-allocate neighbor buffer on stack
    size_t neighborBuffer[256];
    
    for (size_t i = 0; i < n; ++i) {
        float density = 0.0f;
        
        // Get neighbors
        size_t neighborCount = grid.getNeighborsFast(i, particles.positions, neighborBuffer, 256);
        
        // Sum contributions from all neighbors
        const glm::vec2& pi = particles.positions[i];
        for (size_t k = 0; k < neighborCount; ++k) {
            const glm::vec2& pj = particles.positions[neighborBuffer[k]];
            float dx = pi.x - pj.x;
            float dy = pi.y - pj.y;
            float r2 = dx * dx + dy * dy;
            
            // Poly6 kernel using squared distance (avoids sqrt!)
            // W(r) = C * (h² - r²)³
            if (r2 < h2) {
                float diff = h2 - r2;
                float diff3 = diff * diff * diff;
                density += params.m * poly6Coeff * diff3;
            }
        }
        
        // Self-contribution
        density += selfContribution;
        
        particles.densities[i] = density;
    }
}

glm::vec3 densityToColor(float density, float rho0) {
    float t = (density - rho0 * 0.8f) / (rho0 * 0.4f);
    t = std::max(0.0f, std::min(1.0f, t));
    
    glm::vec3 lowColor(0.0f, 0.3f, 1.0f);
    glm::vec3 highColor(1.0f, 0.3f, 0.0f);
    
    return lowColor * (1.0f - t) + highColor * t;
}

} // namespace sph
