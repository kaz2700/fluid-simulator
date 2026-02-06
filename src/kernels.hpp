#ifndef KERNELS_HPP
#define KERNELS_HPP

#include <glm/glm.hpp>

namespace sph {

// Smoothing kernel functions for SPH
class Kernels {
public:
    // Smoothing length (interaction radius)
    static constexpr float DEFAULT_H = 0.1f;
    
    // Poly6 kernel for density calculation
    // W_poly6(r, h) = 315/(64*π*h⁹) * (h² - r²)³ for r ≤ h
    static float W_poly6(float r, float h);
    
    // Spiky kernel gradient for pressure forces
    // ∇W_spiky(r, h) = -45/(π*h⁶) * (h - r)² * (r/|r|) for r ≤ h
    static glm::vec2 gradW_spiky(const glm::vec2& r_vec, float h);
    
    // Viscosity kernel Laplacian
    // ∇²W_viscosity(r, h) = 45/(π*h⁶) * (h - r) for r ≤ h
    static float laplacianW_viscosity(float r, float h);
};

} // namespace sph

#endif // KERNELS_HPP
