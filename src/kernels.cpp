#include "kernels.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace sph {

float Kernels::W_poly6(float r, float h) {
    if (r > h) return 0.0f;
    
    float h2 = h * h;
    float r2 = r * r;
    float diff = h2 - r2;
    
    // 315/(64*π*h⁹)
    float coeff = 315.0f / (64.0f * M_PI * std::pow(h, 9));
    
    return coeff * std::pow(diff, 3);
}

glm::vec2 Kernels::gradW_spiky(const glm::vec2& r_vec, float h) {
    float r = glm::length(r_vec);
    
    if (r > h || r < 1e-6f) {
        return glm::vec2(0.0f, 0.0f);
    }
    
    // -45/(π*h⁶) * (h - r)² / r
    float coeff = -45.0f / (M_PI * std::pow(h, 6));
    float term = std::pow(h - r, 2) / r;
    
    return coeff * term * r_vec;
}

float Kernels::laplacianW_viscosity(float r, float h) {
    if (r > h) return 0.0f;
    
    // 45/(π*h⁶) * (h - r)
    float coeff = 45.0f / (M_PI * std::pow(h, 6));
    
    return coeff * (h - r);
}

} // namespace sph
