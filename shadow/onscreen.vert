#version 450 core

layout (location= 0) in vec3 pos_in;
layout (location= 1) in vec3 normal_in;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 model;
    mat4 normal;
    mat4 projection;
    mat4 clip;
} ubo_in;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out vec4 world_pos_out;
layout (location = 1) out vec3 world_normal_out;

void main() {
    world_normal_out = (ubo_in.normal * vec4(normal_in, 1.f)).xyz;
    world_pos_out = ubo_in.model * vec4(pos_in, 1.f);
    gl_Position = ubo_in.clip * ubo_in.projection * ubo_in.view * world_pos_out;
}
