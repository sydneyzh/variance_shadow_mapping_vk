#version 450 core

layout(triangles) in;

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

layout(location = 0) in vec3 world_pos_in[];
layout(location = 1) in uint layer_mask_in[];

struct View_positions {
    vec4 positions[6];
};
layout (location = 2) in View_positions view_positions_in[];

layout(triangle_strip, max_vertices = 18) out;
out gl_PerVertex
{
    vec4 gl_Position;
};
layout(location = 0) out vec3 world_pos_out;

void emit(uint flag, int view_idx)
{
    vec4 pos0 = ubo_in.clip * plight_info_in.projection * view_positions_in[0].positions[view_idx];
    vec4 pos1 = ubo_in.clip * plight_info_in.projection * view_positions_in[1].positions[view_idx];
    vec4 pos2 = ubo_in.clip * plight_info_in.projection * view_positions_in[2].positions[view_idx];

    if (flag > 0) {
	gl_Position = pos0;
	world_pos_out = world_pos_in[0];
	EmitVertex();

	gl_Position = pos1;
	world_pos_out = world_pos_in[1];
	EmitVertex();

	gl_Position = pos2;
	world_pos_out = world_pos_in[2];
	EmitVertex();

	EndPrimitive();
    }
}

void main()
{
    uint layer_flag_0 = (layer_mask_in[0] & 0x01) |
	(layer_mask_in[1] & 0x01) |
	(layer_mask_in[2] & 0x01);

    uint layer_flag_1 = (layer_mask_in[0] & 0x02) |
	(layer_mask_in[1] & 0x02) |
	(layer_mask_in[2] & 0x02);

    uint layer_flag_2 = (layer_mask_in[0] & 0x04) |
	(layer_mask_in[1] & 0x04) |
	(layer_mask_in[2] & 0x04);

    uint layer_flag_3 = (layer_mask_in[0] & 0x08) |
	(layer_mask_in[1] & 0x08) |
	(layer_mask_in[2] & 0x08);

    uint layer_flag_4 = (layer_mask_in[0] & 0x16) |
	(layer_mask_in[1] & 0x16) |
	(layer_mask_in[2] & 0x16);

    uint layer_flag_5 = (layer_mask_in[0] & 0x64) |
	(layer_mask_in[1] & 0x64) |
	(layer_mask_in[2] & 0x64);

    gl_Layer = 0;
    emit(layer_flag_0, 0);

    gl_Layer = 1;
    emit(layer_flag_1, 1);

    gl_Layer = 2;
    emit(layer_flag_2, 2);

    gl_Layer = 3;
    emit(layer_flag_3, 3);

    gl_Layer = 4;
    emit(layer_flag_4, 4);

    gl_Layer = 5;
    emit(layer_flag_5, 5);
}
