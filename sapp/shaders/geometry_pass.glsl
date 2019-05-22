@ctype mat4 mat4f_t
@ctype vec4 vec4f_t
@ctype vec3 vec3f_t
@ctype vec2 vec2f_t

@vs geo_vs
layout(binding=0) uniform vs_params {
  mat4 view_proj;
  vec4 light;
  vec3 eye_pos;
};

in vec3 vertex_pos;
in vec3 vertex_norm;
in vec2 vertex_uv;

in vec4 instance_color;
in mat4 instance_pose;
in mat4 instance_normal;

out vec3 world_position;
out vec3 world_normal;
out vec3 world_eyepos;
out vec3 world_lightdir;
out vec4 color;
out vec2 uv;

void main() {
  vec4 position = vec4(vertex_pos, 1.0);
  world_position = vec4(instance_pose * position).xyz;
  world_normal = vec4(instance_normal * vec4(vertex_norm, 0.0)).xyz;
  world_eyepos = eye_pos;
  world_lightdir = -light.xyz;
  color = instance_color;
  uv = vertex_uv;
  gl_Position = view_proj * instance_pose * position;
}
@end

@fs geo_fs
layout(binding=0) uniform fs_params {
  vec4 ambient_spec;
};

layout(binding=0) uniform sampler2D albedo_rough;

in vec3 world_position;
in vec3 world_normal;
in vec3 world_eyepos;
in vec3 world_lightdir;
in vec4 color;
in vec2 uv;
out vec4 frag_color;

vec3 light(vec3 base_color, vec3 eye_vec, vec3 normal, vec3 light_vec) {
  float n_dot_l = max(dot(normal, light_vec), 0.0);
  vec3 diff = n_dot_l * base_color;
  float spec_power = ambient_spec.w;
  vec3 r = reflect(-light_vec, normal);
  float r_dot_v = max(dot(r, eye_vec), 0.0);
  float spec = pow(r_dot_v, spec_power) * n_dot_l;
  vec3 ambient = ambient_spec.xyz;
  return ambient + diff + vec3(spec,spec,spec);
}

void main() {
  vec3 eye_vec = normalize(world_eyepos - world_position);
  vec3 nrm = normalize(world_normal);
  vec3 light_dir = normalize(world_lightdir);
  vec3 albedo = texture(albedo_rough, uv).xyz;
  vec3 base_color = albedo * color.xyz;
  frag_color = vec4(light(base_color, eye_vec, nrm, light_dir), 1.0);
}
@end

@program geometry_pass geo_vs geo_fs
