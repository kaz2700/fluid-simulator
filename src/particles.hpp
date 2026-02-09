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

    // Phase 12: Memory optimization - reserve capacity to prevent reallocations
    void reserve(size_t capacity) {
        positions.reserve(capacity);
        velocities.reserve(capacity);
        accelerations.reserve(capacity);
        densities.reserve(capacity);
        pressures.reserve(capacity);
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

    // Phase 11: Add a single particle at position
    void addParticle(const glm::vec2& pos, const glm::vec2& vel = glm::vec2(0.0f)) {
        positions.push_back(pos);
        velocities.push_back(vel);
        accelerations.push_back(glm::vec2(0.0f));
        densities.push_back(0.0f);
        pressures.push_back(0.0f);
    }

    // Phase 11: Remove particles within radius of position
    void removeParticlesNear(const glm::vec2& pos, float radius) {
        float radiusSq = radius * radius;
        size_t writeIdx = 0;
        
        for (size_t i = 0; i < positions.size(); ++i) {
            glm::vec2 diff = positions[i] - pos;
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq > radiusSq) {
                // Keep this particle
                if (writeIdx != i) {
                    positions[writeIdx] = positions[i];
                    velocities[writeIdx] = velocities[i];
                    accelerations[writeIdx] = accelerations[i];
                    densities[writeIdx] = densities[i];
                    pressures[writeIdx] = pressures[i];
                }
                ++writeIdx;
            }
        }
        
        // Resize to remove deleted particles
        resize(writeIdx);
    }

    // Phase 11: Clear all particles
    void clear() {
        positions.clear();
        velocities.clear();
        accelerations.clear();
        densities.clear();
        pressures.clear();
    }
};

#endif
