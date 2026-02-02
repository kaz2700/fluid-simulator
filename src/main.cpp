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

struct Renderer {
    GLuint shaderProgram;
    GLuint VAO, VBO, instanceVBO;

    Renderer() {
        createShaders("shaders/basic.vert", "shaders/basic.frag");
        createBuffers();
    }

    ~Renderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &instanceVBO);
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
        glGenBuffers(1, &instanceVBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);

        glBindVertexArray(0);
    }

    void render(const Particles& particles, const glm::mat4& projection) {
        glUseProgram(shaderProgram);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniform2f(glGetUniformLocation(shaderProgram, "uParticleSize"), 0.01f, 0.01f);

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, particles.positions.size() * sizeof(glm::vec2), particles.positions.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particles.size());
        glBindVertexArray(0);
    }

private:
    char* loadShader(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open shader file: " << path << std::endl;
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
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    void checkProgramErrors(GLuint program) {
        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
};

int main() {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    std::cout << "Creating window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(800, 600, "SPH 2D Simulator", nullptr, nullptr);
        if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glViewport(0, 0, 800, 600);

    Particles particles;
    particles.spawnGrid(100, 100, 0.025f, -0.5f, -0.5f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velDist(-0.5f, 0.5f);

    for (size_t i = 0; i < particles.size(); ++i) {
        particles.velocities[i] = glm::vec2(velDist(gen), velDist(gen));
    }

    Physics physics(0.016f, -9.81f, 0.8f);

    float smoothingLength = 0.08f;
    SpatialHash spatialHash(smoothingLength, 2.0f, 2.0f, -1.0f, -1.0f);
    std::vector<size_t> neighbors;

    Renderer renderer;

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    double lastTime = glfwGetTime();
    double frameCount = 0;
    double simTime = 0.0;
    double neighborSearchTime = 0.0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto neighborStart = std::chrono::high_resolution_clock::now();
        spatialHash.update(particles.positions);
        size_t totalNeighbors = 0;
        for (size_t i = 0; i < particles.size(); ++i) {
            spatialHash.getNeighbors(i, particles.positions, neighbors);
            totalNeighbors += neighbors.size();
        }
        auto neighborEnd = std::chrono::high_resolution_clock::now();
        neighborSearchTime = std::chrono::duration<double, std::milli>(neighborEnd - neighborStart).count();

        physics.velocityVerletStep1(particles);
        physics.handleBoundaries(particles, -0.95f, 0.95f, -0.95f, 0.95f);
        physics.velocityVerletStep2(particles);

        renderer.render(particles, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameCount++;
        simTime += 0.016f;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            std::cout << "FPS: " << frameCount << " | Sim Time: " << simTime << "s | Particles: " << particles.size() 
                      << " | Avg Neighbors: " << (totalNeighbors / (particles.size() ? particles.size() : 1))
                      << " | Neighbor Search: " << neighborSearchTime << "ms" << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }
    }

    glfwTerminate();
    return 0;
}
