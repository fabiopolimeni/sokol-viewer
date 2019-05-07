#pragma once
/**
 * Shaders code
 */

#if defined(SOKOL_GLCORE33)
static const char* vs_src =
"#version 330\n"
"uniform mat4 camera_proj;\n"
"uniform mat4 camera_view;\n"
"uniform mat4 scene_model;\n"
"uniform vec4 light;\n"
"in vec3 vertex_pos;\n"
"in vec3 vertex_norm;\n"
"in vec2 vertex_uv;\n"
"in mat4 instance_pose;\n"
"in vec4 instance_color;\n"
"out vec3 world_position;\n"
"out vec3 world_normal;\n"
"out vec3 world_eyepos;\n"
"out vec3 world_lightdir;\n"
"out vec4 color;\n"
"out vec2 uv;\n"
"void main() {\n"
"  mat4 model = scene_model * instance_pose;\n"
"  world_position = vec4(model * vertex_pos).xyz;\n"
"  world_normal = vec4(model * vec4(vertex_norm, 0.0)).xyz;\n"
"  world_eyepos = camera_view[3].xyz;\n"
"  world_lightdir = light.xyz;\n"
"  color = instance_color;\n"
"  uv = vertex_uv;\n"
"  mat4 mvp = camera_proj * camera_view * model;\n"
"  gl_Position = mvp * vertex_pos;\n"
"}\n";
static const char* fs_src =
"#version 330\n"
"uniform sampler2D albedo_tex;\n"
"in vec3 world_normal;\n"
"in vec3 world_position;\n"
"in vec3 world_eyepos;\n"
"in vec3 world_lightdir;\n"
"in vec4 color;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"vec3 light(vec3 base_color, vec3 eye_vec, vec3 normal, vec3 light_vec) {\n"
"  float n_dot_l = max(dot(normal, light_vec), 0.0);\n"
"  float diff = n_dot_l * base_color;\n"
"  float spec_power = 16.0;\n"
"  vec3 r = reflect(-light_vec, normal);\n"
"  float r_dot_v = max(dot(r, eye_vec), 0.0);\n"
"  float spec = pow(r_dot_v, spec_power) * n_dot_l;"
"  return diff + vec3(spec,spec,spec);"
"}\n"
"void main() {\n"
"  vec3 eye_vec = normalize(world_eyepos - world_position);\n"
"  vec3 nrm = normalize(world_normal);\n"
"  vec3 light_dir = normalize(world_lightdir);\n"
"  vec3 albedo = texture(albedo_tex, uv).xyz;\n"
"  frag_color = vec4(light(albedo * color.xyz, eye_vec, nrm, light_dir), 1.0);\n"
"}\n";
#elif defined(SOKOL_GLES3) || defined(SOKOL_GLES2)
static const char* vs_src =
"uniform mat4 mvp;\n"
"uniform mat4 model;\n"
"uniform vec4 shape_color;\n"
"uniform vec4 light_dir;\n"
"uniform vec4 eye_pos;\n"
"attribute vec4 pos;\n"
"attribute vec3 norm;\n"
"varying vec3 world_position;\n"
"varying vec3 world_normal;\n"
"varying vec3 world_eyepos;\n"
"varying vec3 world_lightdir;\n"
"varying vec4 color;\n"
"void main() {\n"
"  gl_Position = mvp * pos;\n"
"  world_position = vec4(model * pos).xyz;\n"
"  world_normal = vec4(model * vec4(norm, 0.0)).xyz;\n"
"  world_eyepos = eye_pos.xyz;\n"
"  world_lightdir = light_dir.xyz;\n"
"  color = shape_color;\n"
"}\n";
static const char* fs_src =
"precision mediump float;\n"
"uniform samplerCube tex;\n"
"varying vec3 world_normal;\n"
"varying vec3 world_position;\n"
"varying vec3 world_eyepos;\n"
"varying vec3 world_lightdir;\n"
"varying vec4 color;\n"
"vec3 light(vec3 base_color, vec3 eye_vec, vec3 normal, vec3 light_vec) {\n"
"  float ambient = 0.25;\n"
"  float n_dot_l = max(dot(normal, light_vec), 0.0);\n"
"  float diff = n_dot_l + ambient;\n"
"  float spec_power = 16.0;\n"
"  vec3 r = reflect(-light_vec, normal);\n"
"  float r_dot_v = max(dot(r, eye_vec), 0.0);\n"
"  float spec = pow(r_dot_v, spec_power) * n_dot_l;"
"  return base_color * (diff+ambient) + vec3(spec,spec,spec);"
"}\n"
"void main() {\n"
"  vec3 eye_vec = normalize(world_eyepos - world_position);\n"
"  vec3 nrm = normalize(world_normal);\n"
"  vec3 light_dir = normalize(world_lightdir);\n"
"  vec3 refl_vec = normalize(world_position);\n"
"  vec3 refl_color = textureCube(tex, refl_vec).xyz;\n"
"  gl_FragColor = vec4(light(refl_color * color.xyz, eye_vec, nrm, light_dir), 1.0);\n"
"}\n";
#endif
