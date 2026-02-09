#ifndef SPATIAL_HPP
#define SPATIAL_HPP

#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <cstdint>

// Phase 12: Optimized spatial hash with version counter to avoid clearing
class SpatialHash {
public:
    SpatialHash(float smoothingLength, float domainWidth, float domainHeight, float originX = 0.0f, float originY = 0.0f);
    
    void update(const std::vector<glm::vec2>& positions);
    
    void getNeighbors(size_t particleIndex, const std::vector<glm::vec2>& positions, std::vector<size_t>& neighbors) const;
    
    // Fast version using pre-allocated buffer (returns count)
    size_t getNeighborsFast(size_t particleIndex, const std::vector<glm::vec2>& positions, 
                            size_t* buffer, size_t bufferSize) const;
    
    void clear();

private:
    float cellSize;
    float domainWidth;
    float domainHeight;
    float originX;
    float originY;
    
    int gridCols;
    int gridRows;
    
    // Phase 12: Optimized grid structure with version counter
    struct GridCell {
        std::vector<size_t> particles;
        uint32_t version;  // Version counter to track valid entries
        
        GridCell() : version(0) {}
    };
    
    std::vector<GridCell> grid;
    uint32_t currentVersion;
    
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
    
    // Phase 12: Get cell with version checking
    inline GridCell* getCell(int idx) {
        if (idx < 0 || idx >= static_cast<int>(grid.size())) return nullptr;
        auto& cell = grid[idx];
        if (cell.version != currentVersion) {
            // Cell is from a previous frame, clear it
            cell.particles.clear();
            cell.version = currentVersion;
        }
        return &cell;
    }
};

#endif
