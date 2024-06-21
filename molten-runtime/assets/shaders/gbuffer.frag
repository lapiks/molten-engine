#version 330 core

const int MAX_RAY_STEPS = 64;

layout (location = 0) out vec3 o_pos;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out vec4 o_color;

in vec2 io_uv;
in vec3 io_pos;
in vec3 io_normal;
in vec3 io_ray_pos;
in vec3 io_ray_dir;

uniform sampler3D u_vox_model;
uniform mat4 u_view;
uniform vec3 u_model_dim;

struct Ray {
  vec3 pos;
  vec3 dir;
};

vec3 ray_at(Ray ray, float t) {
  return ray.pos + ray.dir * t;
};

struct Box {
  vec3 min_bound;
  vec3 max_bound;
};

bool ray_box_intersection(Ray ray, Box box, inout float t_min, inout float t_max) {
    float t_y_min, t_y_max, t_z_min, t_z_max;
    float x_inv_dir = 1. / ray.dir.x;
    if (x_inv_dir >= 0) {
        t_min = (box.min_bound.x - ray.pos.x) * x_inv_dir;
        t_max = (box.max_bound.x - ray.pos.x) * x_inv_dir;
    } else {
        t_min = (box.max_bound.x - ray.pos.x) * x_inv_dir;
        t_max = (box.min_bound.x - ray.pos.x) * x_inv_dir;
    }

    float y_inv_dir = 1 / ray.dir.y;
    if (y_inv_dir >= 0) {
        t_min = (box.min_bound.y - ray.pos.y) * y_inv_dir;
        t_y_max = (box.max_bound.y - ray.pos.y) * y_inv_dir;
    } else {
        t_y_min = (box.max_bound.y - ray.pos.y) * y_inv_dir;
        t_y_max = (box.min_bound.y - ray.pos.y) * y_inv_dir;
    }

    if (t_min > t_y_max || t_min > t_max) return false;
    if (t_y_min > t_min) t_min = t_y_min;
    if (t_y_max < t_max) t_max = t_y_max;

    float z_inv_dir = 1 / ray.dir.z;
    if (z_inv_dir >= 0) {
        t_z_min = (box.min_bound.z - ray.pos.z) * z_inv_dir;
        t_z_max = (box.max_bound.z - ray.pos.z) * z_inv_dir;
    } else {
        t_z_min = (box.max_bound.z - ray.pos.z) * z_inv_dir;
        t_z_max = (box.min_bound.z - ray.pos.z) * z_inv_dir;
    }

    if (t_min > t_z_max || t_z_min > t_max) return false;
    if (t_z_min > t_min) t_min = t_z_min;
    if (t_z_max < t_max) t_max = t_z_max;
    //return (tMin < t1 && tMax > t0);
    return true;
}

vec3 to_tex_coord(ivec3 voxel_coord) {
  return (voxel_coord * 2 + 1) / (2 * u_model_dim);
}

void main() {
  Ray ray = Ray(
    io_ray_pos,
    normalize(io_ray_dir)
  );

  Box box = Box(
    vec3(0.),
    u_model_dim
  );

  float t_min_box = 0.;
	float t_max_box = 0.;

  if(!ray_box_intersection(ray, box, t_min_box, t_max_box))
    discard;
	
	vec3 ray_start = ray_at(ray, t_min_box + 0.0005);
	ivec3 map_pos = ivec3(floor(ray_start));
	vec3 delta_dist = abs(1. / ray.dir);
	ivec3 ray_step = ivec3(sign(ray.dir));
	vec3 side_dist = (sign(ray.dir) * (vec3(map_pos) - ray.pos) + (sign(ray.dir) * 0.5) + 0.5) * delta_dist; 

	bvec3 mask = bvec3(0);
	
  vec4 voxel_color = vec4(0.0);
  bool hit = false;
	for (int i = 0; i < MAX_RAY_STEPS; i++) {
    voxel_color = texture(u_vox_model, to_tex_coord(map_pos));
		if (voxel_color.r > 0.) {
      hit = true; 
      break;
    }

    mask = lessThanEqual(side_dist.xyz, min(side_dist.yzx, side_dist.zxy));	
			
		side_dist += vec3(mask) * delta_dist;
		map_pos += ivec3(vec3(mask)) * ray_step;
	}

  o_normal = normalize(io_normal);
  o_pos = to_tex_coord(map_pos);

  if (hit) {  
    o_color = voxel_color * 100;
  }
  else
    discard;
}