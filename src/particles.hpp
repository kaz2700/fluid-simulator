#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <vector>
#include <glm/glm.hpp>

struct Particles {
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;
    std::vector<glm::vec2> accelerations;
    std::vector<float> densities;
    std::vector<float> pressures;

    void resize(size_t count) {
        positions.resize(count);
        velocities.resize(count);
        accelerations.resize(count);
        densities.resize(count);
        pressures.resize(count);
    }

    void spawnGrid(int cols, int rows, float spacing, float startX, float startY) {
        size_t count = cols * rows;
        resize(count);
        
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                size_t idx = r * cols + c;
                positions[idx] = glm::vec2(startX + c * spacing, startY + r * spacing);
                velocities[idx] = glm::vec2(0.0f);
                accelerations[idx] = glm::vec2(0.0f);
                densities[idx] = 0.0f;
                pressures[idx] = 0.0f;
            }
        }
    }

    size_t size() const {
        return positions.size();
    }
};

#endif
