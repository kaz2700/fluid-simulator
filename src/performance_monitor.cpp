#include "performance_monitor.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

// Simple 5x7 bitmap font data (ASCII 32-127)
// Each character is 5 bytes (5 columns x 7 rows)
static const unsigned char fontData[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x7F, 0x41, 0x41}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x00, 0x7F, 0x10, 0x28, 0x44}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // ->
};

PerformanceMonitor::PerformanceMonitor()
    : currentFPS(0.0), averageFPS(0.0), frameTimeMs(0.0),
      lastFrameTime(0.0), frameCount(0), startTime(0.0),
      gridTimeMs(0.0f), densityTimeMs(0.0f), pressureCalcTimeMs(0.0f),
      pressureForceTimeMs(0.0f), viscosityTimeMs(0.0f), gravityTimeMs(0.0f), integrationTimeMs(0.0f),
      renderTimeMs(0.0f), adaptiveTimestep(0.016f), isStable(true), currentZoom(1.0f),
      threadCount(1), multiThreadingOn(false),
      gpuModeEnabled(false), gpuAvailable(false),
      shaderProgram(0), VAO(0), VBO(0), fontTexture(0) {
    startTime = glfwGetTime();
    initGL();
    createFontTexture();
}

PerformanceMonitor::~PerformanceMonitor() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &fontTexture);
}

void PerformanceMonitor::initGL() {
    // Create shader program
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
    
    // Create VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

void PerformanceMonitor::createFontTexture() {
    // Create font texture (16x16 grid of 8x8 characters = 128x128 texture)
    const int charWidth = 8;
    const int charHeight = 8;
    const int textureWidth = 128;
    const int textureHeight = 128;
    
    std::vector<unsigned char> textureData(textureWidth * textureHeight, 0);
    
    for (int c = 0; c < 96; c++) {
        int col = c % 16;
        int row = c / 16;
        int baseX = col * charWidth;
        int baseY = row * charHeight;
        
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 7; y++) {
                if (fontData[c][x] & (1 << y)) {
                    int tx = baseX + x;
                    int ty = baseY + (6 - y);
                    textureData[ty * textureWidth + tx] = 255;
                }
            }
        }
    }
    
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, textureData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Setup glyph data
    for (int i = 0; i < 128; i++) {
        if (i >= 32 && i < 128) {
            int c = i - 32;
            int col = c % 16;
            int row = c / 16;
            glyphs[i].u = (col * charWidth) / (float)textureWidth;
            glyphs[i].v = (row * charHeight) / (float)textureHeight;
            glyphs[i].width = 5.0f / textureWidth;
            glyphs[i].height = 7.0f / textureHeight;
        }
    }
}

void PerformanceMonitor::update() {
    double currentTime = glfwGetTime();
    
    if (lastFrameTime > 0) {
        frameTimeMs = (currentTime - lastFrameTime) * 1000.0;
        
        // Add to frame history
        FrameData frame;
        frame.timestamp = currentTime;
        frame.frameTime = frameTimeMs;
        frameHistory.push_back(frame);
        
        // Remove old frames
        while (!frameHistory.empty() && 
               (currentTime - frameHistory.front().timestamp) > HISTORY_DURATION) {
            frameHistory.pop_front();
        }
        
        // Calculate average FPS over the past 10 seconds
        if (!frameHistory.empty()) {
            double totalTime = 0.0;
            for (const auto& f : frameHistory) {
                totalTime += f.frameTime;
            }
            double avgFrameTime = totalTime / frameHistory.size();
            averageFPS = avgFrameTime > 0 ? 1000.0 / avgFrameTime : 0.0;
        }
    }
    
    lastFrameTime = currentTime;
    frameCount++;
    
    // Update current FPS every frame based on instantaneous frame time
    currentFPS = frameTimeMs > 0 ? 1000.0 / frameTimeMs : 0.0;
}

void PerformanceMonitor::updateTiming(float gridTime, float densityTime, float pressureCalcTime,
                                     float pressureForceTime, float viscosityTime, float gravityTime, 
                                     float integrationTime, float renderTime) {
    gridTimeMs = gridTime;
    densityTimeMs = densityTime;
    pressureCalcTimeMs = pressureCalcTime;
    pressureForceTimeMs = pressureForceTime;
    viscosityTimeMs = viscosityTime;
    gravityTimeMs = gravityTime;
    integrationTimeMs = integrationTime;
    renderTimeMs = renderTime;
}

void PerformanceMonitor::render(const glm::mat4& projection, int screenWidth, int screenHeight, size_t particleCount) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Grid: " << gridTimeMs << "ms\n";
    ss << "Density: " << densityTimeMs << "ms\n";
    ss << "Pressure Calc: " << pressureCalcTimeMs << "ms\n";
    ss << "Pressure Forces: " << pressureForceTimeMs << "ms\n";
    ss << "Viscosity: " << viscosityTimeMs << "ms\n";
    ss << "Gravity: " << gravityTimeMs << "ms\n";
    ss << "Integration: " << integrationTimeMs << "ms\n";
    ss << "Render: " << renderTimeMs << "ms\n";
    ss << "Total: " << frameTimeMs << "ms (";
    ss << std::setprecision(1) << currentFPS << " FPS)\n";
    ss << "Particles: " << particleCount << "\n";
    
    // Add elapsed time
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - startTime;
    int minutes = static_cast<int>(elapsedTime) / 60;
    double seconds = elapsedTime - minutes * 60;
    ss << "Time: " << minutes << ":" << std::setprecision(1) << std::setw(4) << std::setfill('0') << seconds << "\n";
    
    // Phase 9: Display adaptive timestep and stability
    ss << std::setprecision(4);
    ss << "Timestep: " << adaptiveTimestep << "ms\n";
    ss << "Status: " << (isStable ? "STABLE" : "UNSTABLE") << "\n";
    
    // Phase 11: Display SPH parameters
    ss << "\n=== Parameters ===\n";
    ss << "Gravity: " << std::setprecision(2) << sphParams.gravity << " m/s^2\n";
    ss << "Viscosity: " << sphParams.mu << "\n";
    ss << "Stiffness: " << sphParams.B << "\n";
    ss << "Rest Density: " << sphParams.rho0 << "\n";
    ss << "Zoom: " << std::setprecision(2) << currentZoom << "x\n";
    
    // Phase 13: Display multi-threading info
    ss << "\n=== Threading ===\n";
    ss << "Threads: " << threadCount << "\n";
    ss << "Mode: " << (multiThreadingOn ? "PARALLEL" : "SEQUENTIAL") << "\n";

    // Phase 14: Display GPU mode info
    ss << "\n=== GPU Mode ===\n";
    ss << "Available: " << (gpuAvailable ? "YES" : "NO") << "\n";
    ss << "Enabled: " << (gpuModeEnabled ? "ON" : "OFF");

    std::string text = ss.str();
    
    // Calculate text position (top-left corner with padding)
    float x = 10.0f;
    float y = 10.0f;
    float scale = 2.0f; // Scale factor for text size
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 1.0f); // White color
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glBindVertexArray(VAO);
    
    // Render each line
    std::istringstream lineStream(text);
    std::string line;
    float lineHeight = 10.0f * scale;
    float currentY = y;
    
    while (std::getline(lineStream, line)) {
        renderText(line, x, currentY, scale);
        currentY += lineHeight;
    }
    
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}

void PerformanceMonitor::renderText(const std::string& text, float x, float y, float scale) {
    float charWidth = 6.0f * scale;
    float charHeight = 8.0f * scale;
    float currentX = x;
    
    std::vector<float> vertices;
    
    for (char c : text) {
        if (c < 32 || c >= 127) continue;
        
        const Glyph& glyph = glyphs[static_cast<unsigned char>(c)];
        
        float xpos = currentX;
        float ypos = y;
        float w = 5.0f * scale;
        float h = 7.0f * scale;
        
        // Add quad vertices (flipped V texture coords to fix upside-down text)
        float quadVertices[6][4] = {
            { xpos,     ypos + h,   glyph.u,                    glyph.v },
            { xpos,     ypos,       glyph.u,                    glyph.v + glyph.height },
            { xpos + w, ypos,       glyph.u + glyph.width,      glyph.v + glyph.height },
            
            { xpos,     ypos + h,   glyph.u,                    glyph.v },
            { xpos + w, ypos,       glyph.u + glyph.width,      glyph.v + glyph.height },
            { xpos + w, ypos + h,   glyph.u + glyph.width,      glyph.v }
        };
        
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 4; j++) {
                vertices.push_back(quadVertices[i][j]);
            }
        }
        
        currentX += charWidth;
    }
    
    if (!vertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);
    }
}

void PerformanceMonitor::renderControls(const glm::mat4& projection, int screenWidth, int screenHeight) {
    // Controls text
    std::string controlsText =
        "=== CONTROLS ===\n"
        "Mouse:\n"
        "  Left Drag  - Add particles\n"
        "  Right Click - Remove particles\n"
        "  Scroll     - Zoom\n\n"
        "Keyboard:\n"
        "  R        - Reset simulation\n"
        "  Space    - Pause/Resume\n"
        "  G        - Toggle gravity\n"
        "  C        - Toggle GPU mode\n"
        "  T        - Toggle multi-threading\n"
        "  1/2/3    - Color modes\n"
        "  ↑/↓      - Adjust gravity\n"
        "  ←/→      - Adjust viscosity\n"
        "  F1-F4    - Scenarios\n"
        "  0        - Default color";
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 0.8f, 0.8f, 0.8f); // Light gray for controls
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glBindVertexArray(VAO);
    
    // Render controls in top-right corner
    float x = screenWidth - 200.0f; // Start 200px from right edge
    float y = 10.0f;
    float scale = 1.5f; // Smaller scale for controls
    float lineHeight = 10.0f * scale;
    
    // Render each line
    std::istringstream lineStream(controlsText);
    std::string line;
    float currentY = y;
    
    while (std::getline(lineStream, line)) {
        renderText(line, x, currentY, scale);
        currentY += lineHeight;
    }
    
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}
