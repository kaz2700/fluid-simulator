#ifndef SPATIAL_HPP
#define SPATIAL_HPP

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

class SpatialHash {
public:
    SpatialHash(float smoothingLength, float domainWidth, float domainHeight, float originX = 0.0f, float originY = 0.0f);
    
    void update(const std::vector<glm::vec2>& positions);
    
    void getNeighbors(size_t particleIndex, const std::vector<glm::vec2>& positions, std::vector<size_t>& neighbors) const;
    
    void clear();

private:
    float cellSize;
    float domainWidth;
    float domainHeight;
    float originX;
    float originY;
    
    int gridCols;
    int gridRows;
    
    std::vector<std::vector<size_t>> grid;
    
    inline int cellIndex(int cx, int cy) const {
        if (cx < 0 || cx >= gridCols || cy < 0 || cy >= gridRows) {
            return -1;
        }
        return cy * gridCols + cx;
    }
    
    inline void getCellCoords(const glm::vec2& pos, int& cx, int& cy) const {
        cx = static_cast<int>((pos.x - originX) / cellSize);
        cy = static_cast<int>((pos.y - originY) / cellSize);
    }
};

#endif
