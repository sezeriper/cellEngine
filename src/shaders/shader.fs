#version 460

out vec4 outColor;
in vec4 gs_color;

void main() {
    outColor = gs_color/255;
}