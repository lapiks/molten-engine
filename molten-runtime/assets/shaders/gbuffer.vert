#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_normal;

out vec3 io_pos;
out vec4 io_color;
out vec2 io_uv;
out vec3 io_normal;

uniform mat4 u_model;
uniform mat4 u_view_proj;

void main() {
  vec4 world_pos = u_model * vec4(a_pos, 1.0);
  io_color = a_color;
  io_uv = a_uv;
  io_pos = world_pos.xyz;

  mat3 normal_mat = transpose(inverse(mat3(u_model)));
  io_normal = normal_mat * a_normal;

  gl_Position = u_view_proj * world_pos;
}