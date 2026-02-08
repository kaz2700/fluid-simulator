#ifndef PERFORMANCE_MONITOR_HPP
#define PERFORMANCE_MONITOR_HPP

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <deque>
#include <string>
#include <chrono>

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    void update();
    void updateTiming(float gridTime, float densityTime, float pressureCalcTime, 
                     float pressureForceTime, float viscosityTime, float gravityTime, 
                     float integrationTime, float renderTime);
    void render(const glm::mat4& projection, int screenWidth, int screenHeight, size_t particleCount = 0);
    
    // Phase 9: Performance baseline tracking
    void startBaselineMeasurement();
    void endBaselineMeasurement();
    void setAdaptiveTimestep(float dt) { adaptiveTimestep = dt; }
    void setStabilityStatus(bool stable) { isStable = stable; }
    
private:
    void initGL();
    void createShaders();
    void createFontTexture();
    void renderText(const std::string& text, float x, float y, float scale);
    
    struct FrameData {
        double timestamp;
        double frameTime;
    };
    
    std::deque<FrameData> frameHistory;
    static constexpr double HISTORY_DURATION = 10.0; // 10 seconds
    
    double currentFPS;
    double averageFPS;
    double frameTimeMs;
    double lastFrameTime;
    int frameCount;
    double startTime;
    
    float gridTimeMs;
    float densityTimeMs;
    float pressureCalcTimeMs;
    float pressureForceTimeMs;
    float viscosityTimeMs;
    float gravityTimeMs;
    float integrationTimeMs;
    float renderTimeMs;
    
    // Phase 9: Adaptive timestep display
    float adaptiveTimestep;
    bool isStable;
    
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint fontTexture;
    
    struct Glyph {
        float u, v;          // Texture coordinates
        float width, height; // Size in texture
    };
    Glyph glyphs[128]; // ASCII glyphs
    
    static constexpr int FONT_TEXTURE_SIZE = 256;
    static constexpr int GLYPH_SIZE = 16;
    
    const char* vertexShaderSource = R"(
        #version 120
        attribute vec2 aPos;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        uniform mat4 uProjection;
        void main() {
            gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
            vTexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 120
        varying vec2 vTexCoord;
        uniform sampler2D uTexture;
        uniform vec3 uColor;
        void main() {
            float alpha = texture2D(uTexture, vTexCoord).r;
            gl_FragColor = vec4(uColor, alpha);
        }
    )";
};

#endif // PERFORMANCE_MONITOR_HPP
