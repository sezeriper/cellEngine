#version 460

layout(location = 0) in ivec2 position;
layout(location = 1) in uvec3 color;

out uvec3 vs_color;

void main() {
    gl_Position = vec4(position, 0.0f, 1.0f);
    vs_color = color;
}