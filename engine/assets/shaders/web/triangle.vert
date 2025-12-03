#version 300 es
precision highp float;

in vec2 position;
in vec3 color;

out vec4 vColor;

void main() {
    gl_Position = vec4(position, 0, 1.0);
    vColor = vec4(color, 1.0);
}