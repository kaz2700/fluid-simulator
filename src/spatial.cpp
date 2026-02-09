#include "spatial.hpp"

SpatialHash::SpatialHash(float smoothingLength, float domainWidth, float domainHeight, float originX, float originY)
    : cellSize(smoothingLength), domainWidth(domainWidth), domainHeight(domainHeight),
      originX(originX), originY(originY), currentVersion(1) {
    
    gridCols = static_cast<int>(std::ceil(domainWidth / cellSize));
    gridRows = static_cast<int>(std::ceil(domainHeight / cellSize));
    
    grid.resize(gridCols * gridRows);
}

void SpatialHash::update(const std::vector<glm::vec2>& positions) {
    // Phase 12: Increment version counter instead of clearing all cells
    // This avoids iterating over all cells and clearing them
    currentVersion++;
    
    // Handle version overflow (very unlikely but possible)
    if (currentVersion == 0) {
        currentVersion = 1;
        for (auto& cell : grid) {
            cell.version = 0;
            cell.particles.clear();
        }
    }
    
    for (size_t i = 0; i < positions.size(); ++i) {
        int cx, cy;
        getCellCoords(positions[i], cx, cy);
        
        int cellIdx = cellIndex(cx, cy);
        if (cellIdx >= 0) {
            auto* cell = getCell(cellIdx);
            if (cell) {
                cell->particles.push_back(i);
            }
        }
    }
}

void SpatialHash::getNeighbors(size_t particleIndex, const std::vector<glm::vec2>& positions, std::vector<size_t>& neighbors) const {
    neighbors.clear();
    
    const glm::vec2& pos = positions[particleIndex];
    int cx, cy;
    getCellCoords(pos, cx, cy);
    
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int cellIdx = cellIndex(cx + dx, cy + dy);
            
            if (cellIdx >= 0) {
                // Phase 12: Only access cells from current version
                const auto& cell = grid[cellIdx];
                if (cell.version != currentVersion) continue;
                
                for (size_t neighborIdx : cell.particles) {
                    if (neighborIdx != particleIndex) {
                        glm::vec2 diff = positions[neighborIdx] - pos;
                        float distSq = glm::dot(diff, diff);
                        
                        if (distSq < cellSize * cellSize) {
                            neighbors.push_back(neighborIdx);
                        }
                    }
                }
            }
        }
    }
}

size_t SpatialHash::getNeighborsFast(size_t particleIndex, const std::vector<glm::vec2>& positions,
                                      size_t* buffer, size_t bufferSize) const {
    size_t count = 0;
    const glm::vec2& pos = positions[particleIndex];
    int cx, cy;
    getCellCoords(pos, cx, cy);

    float searchRadiusSq = cellSize * cellSize;

    for (int dy = -1; dy <= 1 && count < bufferSize; ++dy) {
        for (int dx = -1; dx <= 1 && count < bufferSize; ++dx) {
            int cellIdx = cellIndex(cx + dx, cy + dy);

            if (cellIdx >= 0) {
                // Phase 12: Only access cells from current version
                const auto& cell = grid[cellIdx];
                if (cell.version != currentVersion) continue;
                
                for (size_t neighborIdx : cell.particles) {
                    if (neighborIdx != particleIndex && count < bufferSize) {
                        glm::vec2 diff = positions[neighborIdx] - pos;
                        float distSq = glm::dot(diff, diff);

                        if (distSq < searchRadiusSq) {
                            buffer[count++] = neighborIdx;
                        }
                    }
                }
            }
        }
    }
    return count;
}

void SpatialHash::clear() {
    // Phase 12: Instead of clearing all cells, just increment version
    // This is much faster than iterating over all cells
    currentVersion++;
    
    // Handle version overflow
    if (currentVersion == 0) {
        currentVersion = 1;
        for (auto& cell : grid) {
            cell.version = 0;
            cell.particles.clear();
        }
    }
}
