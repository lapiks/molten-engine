#version 330 core

const float MODEL_DIM = 21;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_normal;

out vec3 io_pos;
out vec4 io_color;
out vec2 io_uv;
out vec3 io_normal;
out vec3 io_ray_pos;
out vec3 io_ray_dir;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
  io_color = a_color;
  io_uv = a_uv;
  io_pos = a_pos.xyz;

  vec4 cam_pos = vec4(-u_view[3][0], -u_view[3][1], -u_view[3][2], 1.0);
  io_ray_pos = (vec3(inverse(u_model) * cam_pos) + vec3(0.5)) * MODEL_DIM;
  io_ray_dir = ((a_pos + vec3(0.5)) * MODEL_DIM) - io_ray_pos;

  mat3 normal_mat = transpose(inverse(mat3(u_model)));
  io_normal = normal_mat * a_normal;

  gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
}