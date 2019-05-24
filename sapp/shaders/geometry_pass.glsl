@ctype mat4 mat4f_t
@ctype vec4 vec4f_t
@ctype vec3 vec3f_t
@ctype vec2 vec2f_t

@vs geo_vs
layout(binding=0) uniform vs_params {
  mat4 view_proj;
};

in vec3 vertex_pos;
in vec3 vertex_norm;
in vec2 vertex_uv;

in vec4 instance_color; // rgb: color, w: layer
in vec4 instance_tile;  // xy:scaling, zw:panning
in mat4 instance_pose;
in mat4 instance_normal;

out vec3 world_position;
out vec3 world_normal;

out vec4 color;
out vec3 uv_layer;

void main() {
  vec4 position = vec4(vertex_pos, 1.0);
  world_position = vec4(instance_pose * position).xyz;
  world_normal = vec4(instance_normal * vec4(vertex_norm, 0.0)).xyz;
  color = vec4(instance_color.xyz, 1.0);
  uv_layer = vec3(vertex_uv * instance_tile.xy + instance_tile.zw,
    instance_color.w);
  gl_Position = view_proj * instance_pose * position;
}
@end

@fs geo_fs
layout(binding=0) uniform fs_params {
  vec4 light_color; // xyz: color, w: intensity
  vec4 light_dir;   // xyz: plane normal, w: plane distance
  vec3 eye_pos;     // eye position in world space
};

layout(binding=0) uniform sampler2DArray albedo_transparency;
layout(binding=1) uniform sampler2DArray emissive_specular;
//layout(binding=2) uniform sampler2DArray normal_disp_ao;

in vec3 world_position;
in vec3 world_normal;

in vec4 color;
in vec3 uv_layer;

out vec4 frag_color;

vec3 light(float spec_power, vec3 base_color, vec3 eye_vec,
  vec3 normal, vec3 light_vec, vec3 ambient, vec3 emissive) {
  float n_dot_l = max(dot(normal, light_vec), 0.0);
  vec3 diff = n_dot_l * base_color;
  vec3 r = reflect(-light_vec, normal);
  float r_dot_v = max(dot(r, eye_vec), 0.0);
  float spec = pow(r_dot_v, spec_power) * n_dot_l;
  vec3 lambert = ambient * (diff + vec3(spec,spec,spec));

  // reducing the irradiance condtribution by the emissive factor
  return ((1.0 - emissive) * lambert) + emissive;
}

void main() {
  vec4 color_alpha = texture(albedo_transparency, uv_layer);
  vec4 em_spec = texture(emissive_specular, uv_layer);
  frag_color = vec4(
    light(
      em_spec.w,
      color_alpha.xyz * color.xyz,
      normalize(eye_pos - world_position),
      normalize(world_normal),
      normalize(-light_dir.xyz),
      light_color.xyz,
      em_spec.zyx),
    color_alpha.w);
}
@end

@program geometry_pass geo_vs geo_fs
