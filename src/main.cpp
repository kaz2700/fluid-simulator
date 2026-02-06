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

struct Renderer {
    GLuint shaderProgram;
    GLuint VAO, VBO, instancePosVBO, instanceDensityVBO;
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

        glBindVertexArray(0);
    }

    void render(const Particles& particles, const glm::mat4& projection, float restDensity, bool useDensityColor) {
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform2f(glGetUniformLocation(shaderProgram, "uParticleSize"), 0.015f, 0.015f);
        glUniform1f(glGetUniformLocation(shaderProgram, "uRestDensity"), restDensity);
        glUniform1i(glGetUniformLocation(shaderProgram, "uUseDensityColor"), useDensityColor ? 1 : 0);

        // Update instance position buffer
        glBindBuffer(GL_ARRAY_BUFFER, instancePosVBO);
        glBufferData(GL_ARRAY_BUFFER, particles.positions.size() * sizeof(glm::vec2), particles.positions.data(), GL_DYNAMIC_DRAW);

        // Update instance density buffer
        glBindBuffer(GL_ARRAY_BUFFER, instanceDensityVBO);
        glBufferData(GL_ARRAY_BUFFER, particles.densities.size() * sizeof(float), particles.densities.data(), GL_DYNAMIC_DRAW);

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SPH 2D Simulator - Phase 5: Density", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) {
        glViewport(0, 0, w, h);
    });

    // SPH parameters
    const float h = 0.08f;           // Smoothing length
    const float m = 0.02f;           // Particle mass
    const float rho0 = 1000.0f;      // Rest density
    const float B = 200.0f;          // Stiffness (not used yet in Phase 5)
    const float mu = 0.1f;           // Viscosity (not used yet in Phase 5)
    
    sph::SPHParams sphParams(h, m, rho0, B, mu);
    sph::SPHSolver sphSolver(sphParams);

    Particles particles;
    particles.spawnGrid(100, 100, 0.025f, -0.5f, -0.5f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velDist(-0.1f, 0.1f);

    for (size_t i = 0; i < particles.size(); ++i) {
        particles.velocities[i] = glm::vec2(velDist(gen), velDist(gen));
    }

    Physics physics(0.016f, -9.81f, 0.8f);

    SpatialHash spatialHash(h, 2.0f, 2.0f, -1.0f, -1.0f);
    std::vector<size_t> neighbors;

    Renderer renderer;
    PerformanceMonitor perfMonitor;

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    bool useDensityColor = true;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update spatial hash
        spatialHash.update(particles.positions);
        
        // Compute densities using SPH
        sphSolver.computeDensities(particles, spatialHash);

        // Physics integration
        physics.velocityVerletStep1(particles);
        physics.handleBoundaries(particles, -1.0f, 1.0f, -1.0f, 1.0f);
        physics.velocityVerletStep2(particles);

        // Render with density-based coloring
        renderer.render(particles, projection, rho0, useDensityColor);

        perfMonitor.update();
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 textProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        perfMonitor.render(textProjection, width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Toggle density coloring with 'D' key
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            useDensityColor = !useDensityColor;
        }
    }

    glfwTerminate();
    return 0;
}
