#version 450 core

layout(location = 0) in vec2 position;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = vec4(1.0, 0.0, 0.0, 1.0);
}