#version 330 core

layout (location = 0) out vec3 o_pos;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out vec4 o_color;

in vec2 io_uv;
in vec3 io_pos;
in vec3 io_normal;

uniform sampler2D u_tex;

void main() {    
    o_pos = io_pos;
    o_normal = normalize(io_normal);
    o_color = texture(u_tex, io_uv);
}