#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aInstPos;

uniform mat4 uProjection;
uniform vec2 uParticleSize;

out vec3 vColor;

void main() {
    vec2 position = aPos * uParticleSize + aInstPos;
    gl_Position = uProjection * vec4(position, 0.0, 1.0);
    vColor = vec3(0.2, 0.4, 0.8);
}
