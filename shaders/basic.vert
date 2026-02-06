#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aInstPos;
layout(location = 2) in float aDensity;

uniform mat4 uProjection;
uniform vec2 uParticleSize;
uniform float uRestDensity;
uniform bool uUseDensityColor;

out vec3 vColor;

vec3 densityToColor(float density, float rho0) {
    float t = (density - rho0 * 0.8) / (rho0 * 0.4);
    t = clamp(t, 0.0, 1.0);
    vec3 lowColor = vec3(0.0, 0.3, 1.0);   // Blue
    vec3 highColor = vec3(1.0, 0.3, 0.0);  // Red
    return mix(lowColor, highColor, t);
}

void main() {
    vec2 position = aPos * uParticleSize + aInstPos;
    gl_Position = uProjection * vec4(position, 0.0, 1.0);
    
    if (uUseDensityColor) {
        vColor = densityToColor(aDensity, uRestDensity);
    } else {
        vColor = vec3(0.0, 1.0, 0.0);  // Default green
    }
}
