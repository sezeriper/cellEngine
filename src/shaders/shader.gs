#version 460

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

const float offset = 0.9f;

in uvec3 vs_color[];
out vec4 gs_color;

uniform mat4 projection;

void makeRect(vec4 position) {
    gl_Position = projection * (position + vec4(0.0f, 0.0f, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(0.0f, offset, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(offset, 0.0f, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(offset, offset, 0.0f, 0.0f));
    EmitVertex();

    EndPrimitive();
}

void main() {
    gs_color = vec4(vs_color[0], 1.0f);
    makeRect(gl_in[0].gl_Position);
}