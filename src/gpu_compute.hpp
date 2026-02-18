#ifndef GPU_COMPUTE_HPP
#define GPU_COMPUTE_HPP

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <cmath>

// Phase 14: GPU Compute Manager for SPH simulation
class GPUCompute {
public:
    struct GPUParticle {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec2 acceleration;
        float density;
        float pressure;
        float padding[2];  // Ensure 32-byte alignment
    };
    
    struct GPUParams {
        float h;           // Smoothing length
        float m;           // Particle mass
        float rho0;        // Rest density
        float B;           // Stiffness
        float mu;          // Viscosity
        float gamma;       // Pressure exponent
        float dt;          // Time step
        float gravity;     // Gravity
        int numParticles;  // Number of particles
        float domainMin;   // Domain boundary min
        float domainMax;   // Domain boundary max
        float damping;     // Boundary damping
        float padding;
    };

    GPUCompute();
    ~GPUCompute();
    
    // Initialize GPU compute with maximum particle capacity
    bool initialize(size_t maxParticles);
    
    // Upload particle data to GPU
    void uploadParticles(const std::vector<glm::vec2>& positions,
                         const std::vector<glm::vec2>& velocities,
                         const std::vector<glm::vec2>& accelerations,
                         const std::vector<float>& densities,
                         const std::vector<float>& pressures);
    
    // Download particle data from GPU
    void downloadParticles(std::vector<glm::vec2>& positions,
                           std::vector<glm::vec2>& velocities,
                           std::vector<glm::vec2>& accelerations,
                           std::vector<float>& densities,
                           std::vector<float>& pressures);
    
    // Set simulation parameters
    void setParams(const GPUParams& params);
    
    // Run compute shaders
    void computeDensities();
    void computePressures();
    void computeForces();
    void integrate();
    void applyBoundaries();
    
    // Full simulation step
    void step();
    void step(float dt);
    
    // Get SSBO IDs for rendering
    GLuint getPositionsSSBO() const { return positionsSSBO; }
    GLuint getVelocitiesSSBO() const { return velocitiesSSBO; }
    GLuint getDensitiesSSBO() const { return densitiesSSBO; }
    GLuint getPressuresSSBO() const { return pressuresSSBO; }
    
    // Check if GPU compute is available
    bool isAvailable() const { return available; }
    
    // Get number of particles
    size_t getNumParticles() const { return numParticles; }
    void setNumParticles(size_t n) { numParticles = n; }

private:
    bool available;
    size_t maxParticles;
    size_t numParticles;
    
    // SSBOs
    GLuint positionsSSBO;
    GLuint velocitiesSSBO;
    GLuint accelerationsSSBO;
    GLuint densitiesSSBO;
    GLuint pressuresSSBO;
    GLuint paramsSSBO;
    
    // Compute programs
    GLuint densityProgram;
    GLuint pressureProgram;
    GLuint forcesProgram;
    GLuint integrateProgram;
    GLuint boundaryProgram;
    
    // Helper functions
    GLuint createComputeShader(const char* source);
    GLuint createComputeProgram(const char* source);
    void checkGLError(const char* operation);
    
    // Shader sources
    static const char* densityShaderSource;
    static const char* pressureShaderSource;
    static const char* forcesShaderSource;
    static const char* integrateShaderSource;
    static const char* boundaryShaderSource;
};

#endif // GPU_COMPUTE_HPP
