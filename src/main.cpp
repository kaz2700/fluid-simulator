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

// Phase 10: Color mode enum
enum ColorMode {
    COLOR_DEFAULT = 0,
    COLOR_DENSITY = 1,
    COLOR_VELOCITY = 2,
    COLOR_PRESSURE = 3
};

struct Renderer {
    GLuint shaderProgram;
    GLuint VAO, VBO, instancePosVBO, instanceDensityVBO, instancePressureVBO, instanceVelocityVBO;
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
        glDeleteBuffers(1, &instanceVelocityVBO);
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
        glGenBuffers(1, &instanceVelocityVBO);

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

        // Phase 10: Instance velocity buffer
        glBindBuffer(GL_ARRAY_BUFFER, instanceVelocityVBO);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);

        glBindVertexArray(0);
    }

    // Phase 10: Updated render function with color mode and velocity
    void render(const Particles& particles, const glm::mat4& projection, float restDensity, ColorMode colorMode, float maxVelocity) {
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform2f(glGetUniformLocation(shaderProgram, "uParticleSize"), 0.015f, 0.015f);
        glUniform1f(glGetUniformLocation(shaderProgram, "uRestDensity"), restDensity);
        glUniform1f(glGetUniformLocation(shaderProgram, "uMaxVelocity"), maxVelocity);
        glUniform1i(glGetUniformLocation(shaderProgram, "uColorMode"), colorMode);

        size_t posSize = particles.positions.size() * sizeof(glm::vec2);
        size_t densitySize = particles.densities.size() * sizeof(float);
        size_t pressureSize = particles.pressures.size() * sizeof(float);
        size_t velocitySize = particles.velocities.size() * sizeof(glm::vec2);

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

        // Phase 10: Upload velocity data for velocity-based coloring
        glBindBuffer(GL_ARRAY_BUFFER, instanceVelocityVBO);
        glBufferData(GL_ARRAY_BUFFER, velocitySize, nullptr, GL_DYNAMIC_DRAW); // Orphan the buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, velocitySize, particles.velocities.data());

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

// Phase 10: Simple background grid renderer
struct GridRenderer {
    GLuint shaderProgram;
    GLuint VAO, VBO;

    GridRenderer() {
        const char* vertexShaderSource = R"(
            #version 120
            attribute vec2 aPos;
            uniform mat4 uProjection;
            void main() {
                gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* fragmentShaderSource = R"(
            #version 120
            void main() {
                gl_FragColor = vec4(0.2, 0.2, 0.2, 1.0);  // Dark gray grid lines
            }
        )";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create grid lines
        std::vector<float> gridVertices;
        float gridSize = 2.0f;
        float gridSpacing = 0.2f;
        int numLines = static_cast<int>(gridSize / gridSpacing);

        // Vertical lines
        for (int i = -numLines; i <= numLines; ++i) {
            float x = i * gridSpacing;
            gridVertices.push_back(x);
            gridVertices.push_back(-gridSize);
            gridVertices.push_back(x);
            gridVertices.push_back(gridSize);
        }

        // Horizontal lines
        for (int i = -numLines; i <= numLines; ++i) {
            float y = i * gridSpacing;
            gridVertices.push_back(-gridSize);
            gridVertices.push_back(y);
            gridVertices.push_back(gridSize);
            gridVertices.push_back(y);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    ~GridRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
    }

    void render(const glm::mat4& projection) {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 84);  // 42 lines * 2 vertices each
        glBindVertexArray(0);
    }
};

// Phase 11: Mouse and keyboard interaction state
struct InteractionState {
    bool leftMousePressed = false;
    bool rightMousePressed = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    float zoomLevel = 1.0f;
    bool paused = false;
    bool gravityEnabled = true;
    
    // For continuous particle addition
    float particleAddTimer = 0.0f;
    const float particleAddInterval = 0.05f; // Add particles every 50ms when dragging
};

// Phase 11: Screen to world coordinate conversion
glm::vec2 screenToWorld(double screenX, double screenY, int screenWidth, int screenHeight, 
                        const glm::mat4& projection) {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / screenHeight; // Flip Y
    
    // Convert NDC to world coordinates using inverse projection
    glm::mat4 invProjection = glm::inverse(projection);
    glm::vec4 worldPos = invProjection * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    return glm::vec2(worldPos.x, worldPos.y);
}

// Phase 11: Scenario presets
enum class Scenario {
    DAM_BREAK,
    WATER_DROP,
    DOUBLE_DAM_BREAK,
    FOUNTAIN
};

void spawnScenario(Particles& particles, Scenario scenario, float particleSpacing) {
    particles.clear();
    
    switch (scenario) {
        case Scenario::DAM_BREAK: {
            // Classic dam break - block of water
            int cols = 71;
            int rows = 71;
            particles.spawnGrid(cols, rows, particleSpacing, -0.5f, -0.5f);
            break;
        }
        case Scenario::WATER_DROP: {
            // Circular cluster falling from top
            float centerX = 0.0f;
            float centerY = 0.5f;
            float radius = 0.3f;
            
            for (float y = -radius; y <= radius; y += particleSpacing) {
                for (float x = -radius; x <= radius; x += particleSpacing) {
                    if (x * x + y * y <= radius * radius) {
                        particles.addParticle(glm::vec2(centerX + x, centerY + y), glm::vec2(0.0f, -1.0f));
                    }
                }
            }
            break;
        }
        case Scenario::DOUBLE_DAM_BREAK: {
            // Two fluid blocks colliding
            int cols = 35;
            int rows = 71;
            // Left block
            particles.spawnGrid(cols, rows, particleSpacing, -0.8f, -0.5f);
            // Right block
            size_t offset = particles.size();
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    particles.addParticle(glm::vec2(0.1f + c * particleSpacing, -0.5f + r * particleSpacing));
                }
            }
            break;
        }
        case Scenario::FOUNTAIN: {
            // Empty initially, particles added continuously
            break;
        }
    }
}

int main() {
    glfwSetErrorCallback([](int error, const char* description) {});

    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SPH 2D Simulator - Phase 11: User Interaction", nullptr, nullptr);
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
    
    // Phase 11: Setup interaction state
    InteractionState interaction;

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

    Physics physics(sphParams.dt, sphParams.gravity, sphParams.damping, sphParams.B, sphParams.rho0, sphParams.gamma, sphParams.mu);

    SpatialHash spatialHash(sphParams.h, 2.0f, 2.0f, -1.0f, -1.0f);
    std::vector<size_t> neighbors;

    Renderer renderer;
    GridRenderer gridRenderer;
    PerformanceMonitor perfMonitor;

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Phase 11: Pass SPH parameters to PerformanceMonitor for display
    perfMonitor.setSPHParameters(sphParams);

    // Phase 10: Color mode selection
    ColorMode colorMode = COLOR_DENSITY;
    
    // Key press delay mechanism
    auto lastKeyTime = std::chrono::high_resolution_clock::now();
    const double keyDelay = 0.3; // 300ms delay
    
    // Phase 11: Parameter adjustment step sizes
    const float gravityStep = 1.0f;
    const float viscosityStep = 0.1f;

    int frameCount = 0;
    
    // Phase 11: Set up mouse callbacks
    glfwSetWindowUserPointer(window, &interaction);
    
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* state = static_cast<InteractionState*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            state->leftMousePressed = (action == GLFW_PRESS);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            state->rightMousePressed = (action == GLFW_PRESS);
        }
    });
    
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto* state = static_cast<InteractionState*>(glfwGetWindowUserPointer(window));
        state->lastMouseX = xpos;
        state->lastMouseY = ypos;
    });
    
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* state = static_cast<InteractionState*>(glfwGetWindowUserPointer(window));
        state->zoomLevel *= (yoffset > 0) ? 1.1f : 0.9f;
        state->zoomLevel = glm::clamp(state->zoomLevel, 0.1f, 5.0f);
    });
    
    // Phase 11: Fixed timestep for interaction timing
    const float interactionDt = 0.016f;
    
    while (!glfwWindowShouldClose(window)) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update spatial hash
        auto t1 = std::chrono::high_resolution_clock::now();
        spatialHash.update(particles.positions);
        auto t2 = std::chrono::high_resolution_clock::now();
        
        // Initialize timing variables
        auto t3 = t2, t4 = t2, t5 = t2, t6 = t2, t7 = t2, t8 = t2;
        float adaptiveDt = sphParams.dt;
        bool isStable = true;
        
        // Phase 11: Skip physics if paused
        if (!interaction.paused) {
            // Compute densities using SPH
            sphSolver.computeDensities(particles, spatialHash);
            t3 = std::chrono::high_resolution_clock::now();
            
            // Compute pressures using Tait equation of state
            physics.computePressures(particles);
            t4 = std::chrono::high_resolution_clock::now();

            // Reset accelerations for new frame
            physics.resetAccelerations(particles);

            // Compute pressure forces
            physics.computePressureForces(particles, spatialHash);
            t5 = std::chrono::high_resolution_clock::now();

            // Compute viscosity forces
            physics.computeViscosityForces(particles, spatialHash);
            t6 = std::chrono::high_resolution_clock::now();

            // Apply gravity (only if enabled)
            if (interaction.gravityEnabled) {
                physics.applyGravity(particles);
            }
            t7 = std::chrono::high_resolution_clock::now();

            // Phase 9: Compute adaptive timestep
            adaptiveDt = physics.computeAdaptiveTimestep(particles, sphParams.h);
            physics.setTimestep(adaptiveDt);
            perfMonitor.setAdaptiveTimestep(adaptiveDt);
            
            // Phase 9: Stability checks
            isStable = physics.checkStability(particles) && physics.validateParticleData(particles);
            perfMonitor.setStabilityStatus(isStable);
            
            // Phase 9: Auto-reset if unstable
            if (!isStable) {
                physics.resetSimulationIfUnstable(particles, gridCols, gridRows, gridSpacing, -0.5f, -0.5f);
            }
            
            // Physics integration
            physics.velocityVerletStep1(particles);
            physics.handleBoundaries(particles, -1.0f, 1.0f, -1.0f, 1.0f);
            physics.velocityVerletStep2(particles);
            t8 = std::chrono::high_resolution_clock::now();
        }
        
        // Phase 11: Handle mouse interactions (add/remove particles) - outside pause check
        if (interaction.leftMousePressed) {
            interaction.particleAddTimer += interactionDt;
            if (interaction.particleAddTimer >= interaction.particleAddInterval) {
                interaction.particleAddTimer = 0.0f;
                glm::vec2 worldPos = screenToWorld(interaction.lastMouseX, interaction.lastMouseY, 
                                                   width, height, projection);
                // Add a small cluster of particles
                particles.addParticle(worldPos);
                particles.addParticle(worldPos + glm::vec2(0.02f, 0.0f));
                particles.addParticle(worldPos + glm::vec2(-0.02f, 0.0f));
                particles.addParticle(worldPos + glm::vec2(0.0f, 0.02f));
                particles.addParticle(worldPos + glm::vec2(0.0f, -0.02f));
            }
        }
        
        if (interaction.rightMousePressed) {
            glm::vec2 worldPos = screenToWorld(interaction.lastMouseX, interaction.lastMouseY, 
                                               width, height, projection);
            particles.removeParticlesNear(worldPos, 0.1f); // Remove within 0.1 radius
        }
        
        // Phase 11: Handle fountain scenario - add particles continuously from top center
        if (particles.size() == 0) {
            // If no particles, add some from fountain
            static float fountainTimer = 0.0f;
            fountainTimer += interactionDt;
            if (fountainTimer > 0.1f) {
                fountainTimer = 0.0f;
                particles.addParticle(glm::vec2(0.0f, 0.8f), glm::vec2(0.0f, -3.0f));
            }
        }

        // Phase 11: Update projection with zoom
        float zoom = interaction.zoomLevel;
        glm::mat4 projection = glm::ortho(-1.0f * zoom, 1.0f * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f);

        // Phase 10: Render background grid first
        gridRenderer.render(projection);
        
        // Phase 10: Render particles with color mode
        float maxVelocity = 5.0f;  // Reference max velocity for coloring
        renderer.render(particles, projection, sphParams.rho0, colorMode, maxVelocity);
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
        
        // Phase 11: Update zoom level in display
        perfMonitor.setZoomLevel(interaction.zoomLevel);
        
        // Render performance monitor (left side) and controls (right side)
        perfMonitor.render(textProjection, width, height, particles.size());
        perfMonitor.renderControls(textProjection, width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
        

        
        // Handle key presses with delay
        auto currentTime = std::chrono::high_resolution_clock::now();
        double timeSinceLastKey = std::chrono::duration<double>(currentTime - lastKeyTime).count();
        
        if (timeSinceLastKey > keyDelay) {
            // Phase 10: Color mode selection with keys 1, 2, 3
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
                colorMode = COLOR_DENSITY;
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
                colorMode = COLOR_VELOCITY;
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
                colorMode = COLOR_PRESSURE;
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
                colorMode = COLOR_DEFAULT;
                lastKeyTime = currentTime;
            }
            
            // Keep old D and P keys for backward compatibility
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                colorMode = COLOR_DENSITY;
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                colorMode = COLOR_PRESSURE;
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Reset simulation with R key
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                particles.clear();
                particles.spawnGrid(gridCols, gridRows, gridSpacing, -0.5f, -0.5f);
                interaction.zoomLevel = 1.0f;
                interaction.paused = false;
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Pause/Resume with Space
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                interaction.paused = !interaction.paused;
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Toggle gravity with G
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
                interaction.gravityEnabled = !interaction.gravityEnabled;
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Adjust gravity with Up/Down arrows
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                sphParams.gravity += gravityStep;
                physics.setGravity(sphParams.gravity);
                perfMonitor.setSPHParameters(sphParams);
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                sphParams.gravity -= gravityStep;
                physics.setGravity(sphParams.gravity);
                perfMonitor.setSPHParameters(sphParams);
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Adjust viscosity with Left/Right arrows
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                sphParams.mu += viscosityStep;
                physics.setViscosity(sphParams.mu);
                perfMonitor.setSPHParameters(sphParams);
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                sphParams.mu = std::max(0.0f, sphParams.mu - viscosityStep);
                physics.setViscosity(sphParams.mu);
                perfMonitor.setSPHParameters(sphParams);
                lastKeyTime = currentTime;
            }
            
            // Phase 11: Scenario presets with function keys
            if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
                spawnScenario(particles, Scenario::DAM_BREAK, gridSpacing);
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
                spawnScenario(particles, Scenario::WATER_DROP, gridSpacing);
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
                spawnScenario(particles, Scenario::DOUBLE_DAM_BREAK, gridSpacing);
                lastKeyTime = currentTime;
            }
            if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
                spawnScenario(particles, Scenario::FOUNTAIN, gridSpacing);
                lastKeyTime = currentTime;
            }
        }
    }

    glfwTerminate();
    return 0;
}
