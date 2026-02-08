#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <random>
#include <chrono>
#include "particles.hpp"
#include "physics.hpp"
#include "spatial.hpp"
#include "sph.hpp"
#include "performance_monitor.hpp"

// Simple timer for profiling
struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    const char* name;
    Timer(const char* n) : name(n) {
        start = std::chrono::high_resolution_clock::now();
    }
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float, std::milli>(end - start).count();
        std::cout << name << ": " << duration << " ms" << std::endl;
    }
};

struct Renderer {
    GLuint shaderProgram;
    GLuint VAO, VBO, instancePosVBO, instanceDensityVBO, instancePressureVBO;
    GLuint EBO;

    Renderer() {
        createShaders("shaders/basic.vert", "shaders/basic.frag");
        createBuffers();
    }

    ~Renderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &instancePosVBO);
        glDeleteBuffers(1, &instanceDensityVBO);
        glDeleteBuffers(1, &instancePressureVBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
    }

    void createShaders(const char* vertexPath, const char* fragmentPath) {
        const char* vertexShaderSource = loadShader(vertexPath);
        const char* fragmentShaderSource = loadShader(fragmentPath);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        checkShaderErrors(vertexShader, "VERTEX");

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        checkShaderErrors(fragmentShader, "FRAGMENT");

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        checkProgramErrors(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        delete[] vertexShaderSource;
        delete[] fragmentShaderSource;
    }

    void createBuffers() {
        float particleVertices[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.5f,  0.5f,
            -0.5f,  0.5f
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &instancePosVBO);
        glGenBuffers(1, &instanceDensityVBO);
        glGenBuffers(1, &instancePressureVBO);

        glBindVertexArray(VAO);

        // Vertex buffer (quad geometry)
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Instance position buffer
        glBindBuffer(GL_ARRAY_BUFFER, instancePosVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);

        // Instance density buffer
        glBindBuffer(GL_ARRAY_BUFFER, instanceDensityVBO);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);

        // Instance pressure buffer
        glBindBuffer(GL_ARRAY_BUFFER, instancePressureVBO);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);

        glBindVertexArray(0);
    }

    void render(const Particles& particles, const glm::mat4& projection, float restDensity, bool useDensityColor, bool usePressureColor) {
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform2f(glGetUniformLocation(shaderProgram, "uParticleSize"), 0.015f, 0.015f);
        glUniform1f(glGetUniformLocation(shaderProgram, "uRestDensity"), restDensity);
        glUniform1i(glGetUniformLocation(shaderProgram, "uUseDensityColor"), useDensityColor ? 1 : 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "uUsePressureColor"), usePressureColor ? 1 : 0);

        size_t posSize = particles.positions.size() * sizeof(glm::vec2);
        size_t densitySize = particles.densities.size() * sizeof(float);
        size_t pressureSize = particles.pressures.size() * sizeof(float);

        // Use glBufferSubData to avoid GPU memory reallocation
        glBindBuffer(GL_ARRAY_BUFFER, instancePosVBO);
        glBufferData(GL_ARRAY_BUFFER, posSize, nullptr, GL_DYNAMIC_DRAW); // Orphan the buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, particles.positions.data());

        glBindBuffer(GL_ARRAY_BUFFER, instanceDensityVBO);
        glBufferData(GL_ARRAY_BUFFER, densitySize, nullptr, GL_DYNAMIC_DRAW); // Orphan the buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, densitySize, particles.densities.data());

        glBindBuffer(GL_ARRAY_BUFFER, instancePressureVBO);
        glBufferData(GL_ARRAY_BUFFER, pressureSize, nullptr, GL_DYNAMIC_DRAW); // Orphan the buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, pressureSize, particles.pressures.data());

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particles.size());
        glBindVertexArray(0);
    }

private:
    char* loadShader(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return nullptr;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string str = buffer.str();
        char* result = new char[str.size() + 1];
        strcpy(result, str.c_str());
        return result;
    }

    void checkShaderErrors(GLuint shader, const char* type) {
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        }
    }

    void checkProgramErrors(GLuint program) {
        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
        }
    }
};

int main() {
    glfwSetErrorCallback([](int error, const char* description) {});

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SPH 2D Simulator - Phase 9: Stability & Tuning", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);  // Disable vsync to see true performance

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
        glViewport(0, 0, w, h);
    });

    // Phase 9: Centralized SPH Parameters
    SPHParameters sphParams;
    sphParams.h = 0.08f;              // Smoothing length
    sphParams.m = 0.02f;              // Particle mass
    sphParams.rho0 = 550.0f;          // Rest density (adjusted to match actual particle density ~557)
    sphParams.B = 50.0f;              // Stiffness
    sphParams.mu = 0.1f;              // Viscosity
    sphParams.gamma = 7.0f;           // Pressure exponent
    sphParams.dt = 0.016f;            // Initial time step (16ms = ~60 FPS)
    sphParams.minDt = 0.0001f;        // Minimum time step (0.1ms)
    sphParams.maxDt = 0.01f;          // Maximum time step (10ms)
    sphParams.CFL = 0.4f;             // Courant-Friedrichs-Lewy number
    sphParams.gravity = -9.81f;       // Gravity
    sphParams.damping = 0.8f;         // Boundary damping
    sphParams.adaptiveTimestep = true; // Enable adaptive timestep
    
    sph::SPHParams sphSolverParams(sphParams.h, sphParams.m, sphParams.rho0, sphParams.B, sphParams.mu);
    sph::SPHSolver sphSolver(sphSolverParams);

    Particles particles;
    const int gridCols = 71;
    const int gridRows = 71;
    const float gridSpacing = 0.02f;
    particles.spawnGrid(gridCols, gridRows, gridSpacing, -0.5f, -0.5f);  // ~5,000 particles

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velDist(-0.1f, 0.1f);

    for (size_t i = 0; i < particles.size(); ++i) {
        particles.velocities[i] = glm::vec2(0.0f, 0.0f);  // Zero initial velocity for all particles
    }

    Physics physics(sphParams.dt, sphParams.gravity, sphParams.damping, sphParams.B, sphParams.rho0, sphParams.gamma);

    SpatialHash spatialHash(sphParams.h, 2.0f, 2.0f, -1.0f, -1.0f);
    std::vector<size_t> neighbors;

    Renderer renderer;
    PerformanceMonitor perfMonitor;

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    bool useDensityColor = true;
    bool usePressureColor = false;
    
    // Key press delay mechanism
    auto lastKeyTime = std::chrono::high_resolution_clock::now();
    const double keyDelay = 0.3; // 300ms delay

    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update spatial hash
        auto t1 = std::chrono::high_resolution_clock::now();
        spatialHash.update(particles.positions);
        auto t2 = std::chrono::high_resolution_clock::now();
        
        // Compute densities using SPH
        sphSolver.computeDensities(particles, spatialHash);
        auto t3 = std::chrono::high_resolution_clock::now();
        
        // Compute pressures using Tait equation of state
        physics.computePressures(particles);
        auto t4 = std::chrono::high_resolution_clock::now();

        // Reset accelerations for new frame
        physics.resetAccelerations(particles);

        // Compute pressure forces
        physics.computePressureForces(particles, spatialHash);
        auto t5 = std::chrono::high_resolution_clock::now();

        // Compute viscosity forces
        physics.computeViscosityForces(particles, spatialHash);
        auto t6 = std::chrono::high_resolution_clock::now();

        // Apply gravity
        physics.applyGravity(particles);
        auto t7 = std::chrono::high_resolution_clock::now();

        // Phase 9: Compute adaptive timestep
        float adaptiveDt = physics.computeAdaptiveTimestep(particles, sphParams.h);
        physics.setTimestep(adaptiveDt);
        perfMonitor.setAdaptiveTimestep(adaptiveDt);
        
        // Phase 9: Stability checks
        bool isStable = physics.checkStability(particles) && physics.validateParticleData(particles);
        perfMonitor.setStabilityStatus(isStable);
        
        // Phase 9: Auto-reset if unstable
        if (!isStable) {
            physics.resetSimulationIfUnstable(particles, gridCols, gridRows, gridSpacing, -0.5f, -0.5f);
        }
        
        // Physics integration
        physics.velocityVerletStep1(particles);
        physics.handleBoundaries(particles, -1.0f, 1.0f, -1.0f, 1.0f);
        physics.velocityVerletStep2(particles);
        auto t8 = std::chrono::high_resolution_clock::now();

        // Render with density-based coloring
        renderer.render(particles, projection, sphParams.rho0, useDensityColor, usePressureColor);
        auto t9 = std::chrono::high_resolution_clock::now();

        perfMonitor.update();

        // Calculate timing data
        auto gridTime = std::chrono::duration<float, std::milli>(t2 - t1).count();
        auto densityTime = std::chrono::duration<float, std::milli>(t3 - t2).count();
        auto pressureTime = std::chrono::duration<float, std::milli>(t4 - t3).count();
        auto pressureForceTime = std::chrono::duration<float, std::milli>(t5 - t4).count();
        auto viscosityTime = std::chrono::duration<float, std::milli>(t6 - t5).count();
        auto gravityTime = std::chrono::duration<float, std::milli>(t7 - t6).count();
        auto integrationTime = std::chrono::duration<float, std::milli>(t8 - t7).count();
        auto renderTime = std::chrono::duration<float, std::milli>(t9 - t8).count();
        perfMonitor.updateTiming(gridTime, densityTime, pressureTime, pressureForceTime, viscosityTime, gravityTime, integrationTime, renderTime);
        
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 textProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        perfMonitor.render(textProjection, width, height, particles.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
        

        
        // Handle key presses with delay
        auto currentTime = std::chrono::high_resolution_clock::now();
        double timeSinceLastKey = std::chrono::duration<double>(currentTime - lastKeyTime).count();
        
        if (timeSinceLastKey > keyDelay) {
            // Toggle density coloring with 'D' key
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                useDensityColor = !useDensityColor;
                usePressureColor = false;
                lastKeyTime = currentTime;
            }
            
            // Toggle pressure coloring with 'P' key
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                usePressureColor = !usePressureColor;
                useDensityColor = false;
                lastKeyTime = currentTime;
            }
        }
    }

    glfwTerminate();
    return 0;
}
