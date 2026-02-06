#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "kernels.hpp"

using namespace sph;

bool test_poly6_kernel_boundary() {
    float h = 0.1f;
    float result = Kernels::W_poly6(h, h);
    bool pass = std::abs(result) < 1e-6f;
    std::cout << "  Poly6 at r=h: " << result << " (expected ~0) " << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_poly6_kernel_maximum_at_center() {
    float h = 0.1f;
    float center = Kernels::W_poly6(0.0f, h);
    float half = Kernels::W_poly6(h * 0.5f, h);
    float quarter = Kernels::W_poly6(h * 0.25f, h);
    
    // Center should be maximum, and values should decrease as r increases
    bool pass = (center > quarter) && (quarter > half);
    std::cout << "  Poly6 max at r=0: " << center << " > " << quarter << " > " << half << " " 
              << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_poly6_kernel_zero_outside() {
    float h = 0.1f;
    float result = Kernels::W_poly6(h + 0.01f, h);
    bool pass = std::abs(result) < 1e-6f;
    std::cout << "  Poly6 outside support: " << result << " (expected 0) " << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_poly6_kernel_symmetry() {
    float h = 0.1f;
    float r1 = 0.03f;
    float r2 = -0.03f;
    float result1 = Kernels::W_poly6(r1, h);
    float result2 = Kernels::W_poly6(std::abs(r2), h);
    bool pass = std::abs(result1 - result2) < 1e-6f;
    std::cout << "  Poly6 symmetry: " << result1 << " == " << result2 << " " << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_spiky_gradient_direction() {
    float h = 0.1f;
    glm::vec2 r_vec(0.05f, 0.0f);
    glm::vec2 grad = Kernels::gradW_spiky(r_vec, h);
    
    // Gradient should point in the same direction as r_vec (away from center)
    bool pointsAway = (grad.x < 0.0f) && (std::abs(grad.y) < 1e-6f);
    std::cout << "  Spiky gradient direction: (" << grad.x << ", " << grad.y << ") " 
              << (pointsAway ? "PASS" : "FAIL") << std::endl;
    return pointsAway;
}

bool test_spiky_gradient_magnitude() {
    float h = 0.1f;
    float r1 = 0.02f;
    float r2 = 0.05f;
    float r3 = 0.08f;
    
    glm::vec2 grad1 = Kernels::gradW_spiky(glm::vec2(r1, 0.0f), h);
    glm::vec2 grad2 = Kernels::gradW_spiky(glm::vec2(r2, 0.0f), h);
    glm::vec2 grad3 = Kernels::gradW_spiky(glm::vec2(r3, 0.0f), h);
    
    float mag1 = glm::length(grad1);
    float mag2 = glm::length(grad2);
    float mag3 = glm::length(grad3);
    
    // Magnitude should decrease as r increases
    bool pass = (mag1 > mag2) && (mag2 > mag3);
    std::cout << "  Spiky gradient magnitude: " << mag1 << " > " << mag2 << " > " << mag3 << " "
              << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_spiky_gradient_zero_at_boundary() {
    float h = 0.1f;
    glm::vec2 r_vec(h, 0.0f);
    glm::vec2 grad = Kernels::gradW_spiky(r_vec, h);
    
    bool pass = glm::length(grad) < 1e-6f;
    std::cout << "  Spiky gradient at r=h: (" << grad.x << ", " << grad.y << ") " 
              << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_spiky_gradient_zero_at_center() {
    float h = 0.1f;
    glm::vec2 r_vec(0.0f, 0.0f);
    glm::vec2 grad = Kernels::gradW_spiky(r_vec, h);
    
    bool pass = glm::length(grad) < 1e-6f;
    std::cout << "  Spiky gradient at r=0: (" << grad.x << ", " << grad.y << ") " 
              << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_viscosity_non_negative() {
    float h = 0.1f;
    bool pass = true;
    
    for (float r = 0.0f; r <= h; r += 0.01f) {
        float result = Kernels::laplacianW_viscosity(r, h);
        if (result < 0.0f) {
            pass = false;
            break;
        }
    }
    
    std::cout << "  Viscosity non-negative: " << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_viscosity_zero_at_boundary() {
    float h = 0.1f;
    float result = Kernels::laplacianW_viscosity(h, h);
    bool pass = std::abs(result) < 1e-6f;
    std::cout << "  Viscosity at r=h: " << result << " (expected ~0) " << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_viscosity_maximum_at_center() {
    float h = 0.1f;
    float center = Kernels::laplacianW_viscosity(0.0f, h);
    float half = Kernels::laplacianW_viscosity(h * 0.5f, h);
    float quarter = Kernels::laplacianW_viscosity(h * 0.25f, h);
    
    // Viscosity kernel is linear: W(r) ∝ (h - r), so center > quarter > half
    bool pass = (center > quarter) && (quarter > half);
    std::cout << "  Viscosity max at r=0: " << center << " > " << quarter << " > " << half << " "
              << (pass ? "PASS" : "FAIL") << std::endl;
    return pass;
}

bool test_kernel_values_at_specific_points() {
    float h = 0.1f;
    
    // Test Poly6 at r=0 (should be maximum)
    float poly6_center = Kernels::W_poly6(0.0f, h);
    float expected_poly6_center = (315.0f / (64.0f * M_PI * std::pow(h, 9))) * std::pow(h * h, 3);
    bool poly6_pass = std::abs(poly6_center - expected_poly6_center) < 1e-3f;
    
    // Test viscosity at r=0
    float visc_center = Kernels::laplacianW_viscosity(0.0f, h);
    float expected_visc_center = 45.0f / (M_PI * std::pow(h, 6)) * h;
    bool visc_pass = std::abs(visc_center - expected_visc_center) < 1e-3f;
    
    std::cout << "  Kernel values verification:" << std::endl;
    std::cout << "    Poly6(0): " << poly6_center << " vs expected " << expected_poly6_center << " "
              << (poly6_pass ? "PASS" : "FAIL") << std::endl;
    std::cout << "    Viscosity(0): " << visc_center << " vs expected " << expected_visc_center << " "
              << (visc_pass ? "PASS" : "FAIL") << std::endl;
    
    return poly6_pass && visc_pass;
}

int main() {
    std::cout << "=== SPH Kernel Unit Tests ===" << std::endl << std::endl;
    
    int passed = 0;
    int total = 0;
    
    std::cout << "Poly6 Kernel Tests:" << std::endl;
    total++; if (test_poly6_kernel_boundary()) passed++;
    total++; if (test_poly6_kernel_maximum_at_center()) passed++;
    total++; if (test_poly6_kernel_zero_outside()) passed++;
    total++; if (test_poly6_kernel_symmetry()) passed++;
    
    std::cout << std::endl << "Spiky Gradient Tests:" << std::endl;
    total++; if (test_spiky_gradient_direction()) passed++;
    total++; if (test_spiky_gradient_magnitude()) passed++;
    total++; if (test_spiky_gradient_zero_at_boundary()) passed++;
    total++; if (test_spiky_gradient_zero_at_center()) passed++;
    
    std::cout << std::endl << "Viscosity Laplacian Tests:" << std::endl;
    total++; if (test_viscosity_non_negative()) passed++;
    total++; if (test_viscosity_zero_at_boundary()) passed++;
    total++; if (test_viscosity_maximum_at_center()) passed++;
    
    std::cout << std::endl << "Mathematical Properties Tests:" << std::endl;
    total++; if (test_kernel_values_at_specific_points()) passed++;
    
    std::cout << std::endl << "=== Results: " << passed << "/" << total << " tests passed ===" << std::endl;
    
    return (passed == total) ? 0 : 1;
}
