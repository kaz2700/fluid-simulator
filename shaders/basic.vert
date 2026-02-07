#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aInstPos;
layout(location = 2) in float aDensity;
layout(location = 3) in float aPressure;

uniform mat4 uProjection;
uniform vec2 uParticleSize;
uniform float uRestDensity;
uniform bool uUseDensityColor;
uniform bool uUsePressureColor;

out vec3 vColor;

vec3 densityToColor(float density, float rho0) {
    float t = (density - rho0 * 0.8) / (rho0 * 0.4);
    t = clamp(t, 0.0, 1.0);
    vec3 lowColor = vec3(0.0, 0.3, 1.0);   // Blue
    vec3 highColor = vec3(1.0, 0.3, 0.0);  // Red
    return mix(lowColor, highColor, t);
}

vec3 pressureToColor(float pressure, float maxPressure) {
    // Use logarithmic scaling for better visualization of pressure variations
    float logPressure = log(max(pressure, 0.001) + 1.0);
    float logMax = log(maxPressure + 1.0);
    float t = clamp(logPressure / logMax, 0.0, 1.0);
    
    vec3 lowColor = vec3(0.0, 0.5, 1.0);   // Light blue
    vec3 midColor = vec3(0.0, 1.0, 0.0);   // Green  
    vec3 highColor = vec3(1.0, 1.0, 0.0);  // Yellow
    
    if (t < 0.5) {
        return mix(lowColor, midColor, t * 2.0);
    } else {
        return mix(midColor, highColor, (t - 0.5) * 2.0);
    }
}

void main() {
    vec2 position = aPos * uParticleSize + aInstPos;
    gl_Position = uProjection * vec4(position, 0.0, 1.0);
    
    if (uUseDensityColor) {
        vColor = densityToColor(aDensity, uRestDensity);
    } else if (uUsePressureColor) {
        vColor = pressureToColor(aPressure, 100.0f);  // Max pressure for visualization
    } else {
        vColor = vec3(0.0, 1.0, 0.0);  // Default green
    }
}
