#version 450 core

layout(set = 0, binding = 1) uniform PLight_info
{
    vec3 pos;
    float range;
    mat4 view0;
    mat4 projection;
} plight_info_in;

layout(location = 0) in vec3 world_pos_in;

layout (location = 0) out vec4 frag_color;

void main() {
    float d = distance(world_pos_in.xyz, plight_info_in.pos);
    frag_color.x = d;
    frag_color.y = d * d;
}
