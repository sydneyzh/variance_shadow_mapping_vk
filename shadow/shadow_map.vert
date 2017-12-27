#version 450 core

#define TAN_HALF_FOVY 1.f // tan(45deg)
#define ASP 1.f
#define LIGHT_FRUSTUM_NEAR .1f

layout (location= 0) in vec3 pos_in;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 model;
    mat4 normal;
    mat4 projection;
    mat4 clip;
} ubo_in;

layout(set = 0, binding = 1) uniform PLight_info
{
    vec3 pos;
    float range;
    mat4 view0;
    mat4 projection;
} plight_info_in;

layout (location = 0) out vec3 world_pos_out;
layout (location = 1) out uint layer_mask_out;

struct View_positions {
    vec4 positions[6];
};
layout (location = 2) out View_positions view_positions_out;

uint get_layer_flag(vec4 view_pos, uint flag)
{
    // if view_pos is in frustum, return flag
    // otherwise return 0

    uint res = 1;
    res *= uint(step(LIGHT_FRUSTUM_NEAR, -view_pos.z));
    res *= uint(step(-plight_info_in.range, view_pos.z));

    float ymax = - TAN_HALF_FOVY * view_pos.z;
    float xmax = ymax * ASP;
    res *= uint(step(- xmax, view_pos.x));
    res *= uint(step(view_pos.x, xmax));
    res *- uint(step(- ymax, view_pos.y));
    res *- uint(step(view_pos.y, ymax));

    return res * flag;
}

void main(void)
{
    vec4 world_pos = ubo_in.model * vec4(pos_in, 1.f);
    world_pos_out = world_pos.xyz;

    // posx
    vec4 view0_pos = plight_info_in.view0 * world_pos;
    view0_pos.x *= -1.f;

    // negx
    vec4 view1_pos = vec4(-view0_pos.x, view0_pos.y, -view0_pos.z, 1.f);

    // posy
    vec4 view2_pos = vec4(-view0_pos.z, view0_pos.x, -view0_pos.y, 1.f);

    // negy
    vec4 view3_pos = vec4(-view0_pos.z, -view0_pos.x, view0_pos.y, 1.f);

    // posz
    vec4 view4_pos = vec4(-view0_pos.z, view0_pos.y, view0_pos.x, 1.f);

    // negz
    vec4 view5_pos = vec4(view0_pos.z, view0_pos.y, -view0_pos.x, 1.f);

    layer_mask_out = get_layer_flag(view0_pos, 0x01) |
	get_layer_flag(view1_pos, 0x02) |
	get_layer_flag(view2_pos, 0x04) |
	get_layer_flag(view3_pos, 0x08) |
	get_layer_flag(view4_pos, 0x16) |
	get_layer_flag(view5_pos, 0x64);

    view_positions_out.positions[0] = view0_pos;
    view_positions_out.positions[1] = view1_pos;
    view_positions_out.positions[2] = view2_pos;
    view_positions_out.positions[3] = view3_pos;
    view_positions_out.positions[4] = view4_pos;
    view_positions_out.positions[5] = view5_pos;
}
