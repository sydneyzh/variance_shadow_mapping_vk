#version 450 core
#define PI 3.1415926f

layout (location = 0) in vec2 uv_in;

layout(push_constant) uniform Filter_info{
    vec2 direction;
    float res;
    float inv_res;
} filter_info_in;

layout(set = 0, binding = 0) uniform sampler2D shadow_maps[6];

layout(location = 0) out vec4 frag_color_0;
layout(location = 1) out vec4 frag_color_1;
layout(location = 2) out vec4 frag_color_2;
layout(location = 3) out vec4 frag_color_3;
layout(location = 4) out vec4 frag_color_4;
layout(location = 5) out vec4 frag_color_5;


vec3 wrap_uv(vec2 offset, int face) {
    float res = filter_info_in.res;
    float inv_res = filter_info_in.inv_res;
    vec2 uv_new = uv_in + offset;
    float s = uv_new.x;
    float t = uv_new.y;
    if (s >= 1.f) {
	if (face == 0) { face=5; uv_new.x=s - 1.f; }
	else if (face == 1) { face=4; uv_new.x=s - 1.f; }
	else if (face == 2) { face=0; uv_new.x=1.f - t - inv_res; uv_new.y=s - 1.f; }
	else if (face == 3) { face=0; uv_new.x=t; uv_new.y=2.f - s - inv_res; }
	else if (face == 4) { face=0; uv_new.x=s - 1.f; }
	else if (face == 5) { face=1; uv_new.x=s - 1.f; }
    }
    else if (s < 0.f) {
	if (face == 0) { face=4; uv_new.x=s + 1.f; }
	else if (face == 1) { face=5; uv_new.x=s + 1.f; }
	else if (face == 2) { face=1; uv_new.x=t; uv_new.y=-s - inv_res; }
	else if (face == 3) { face=1; uv_new.x=1.f - t - inv_res; uv_new.y=s + 1.f; }
	else if (face == 4) { face=1; uv_new.x=s + 1.f; }
	else if (face == 5) { face=0; uv_new.x=s + 1.f; }
    }
    else if (t >= 1.f) {
	if (face == 0) { face=3; uv_new.x=2.f - t - inv_res; uv_new.y=s; }
	else if (face == 1) { face=3; uv_new.x=t - 1.f; uv_new.y=1.f - s - inv_res; }
	else if (face == 2) { face=4; uv_new.y=t - 1.f; }
	else if (face == 3) { face=5; uv_new.x=1.f - s - inv_res; uv_new.y=2.f - t - inv_res; }
	else if (face == 4) { face=3; uv_new.y=t - 1.f; }
	else if (face == 5) { face=3; uv_new.x=1.f - s - inv_res; uv_new.y=2.f - t - inv_res; }
    }
    else if (t < 0.f) {
	if (face==0) { face=2; uv_new.x=1.f + t; uv_new.y=1.f - s - inv_res; }
	else if (face == 1) { face=2; uv_new.x=-t - inv_res; uv_new.y=s; }
	else if (face == 2) { face=5; uv_new.x=1.f - s - inv_res; uv_new.y=-t - inv_res; }
	else if (face == 3) { face=4; uv_new.y=t + 1.f; }
	else if (face == 4) { face=2; uv_new.y=t + 1.f; }
	else if (face == 5) { face=2; uv_new.x=1.f - s - inv_res; uv_new.y=-t - inv_res; }
    }

    return vec3(uv_new, float(face));
}

vec4 blur9(int face)
{
    vec2 off1 = 1.411764705882353 * filter_info_in.direction * filter_info_in.inv_res;
    vec2 off2 = 3.2941176470588234 * filter_info_in.direction * filter_info_in.inv_res;
    vec2 off3 = 5.176470588235294 * filter_info_in.direction * filter_info_in.inv_res;

    vec2 moments = texture(shadow_maps[face], uv_in).xy * 0.1964825501511404;

    vec3 uv_face = wrap_uv(off1, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.2969069646728344;

    uv_face = wrap_uv(-off1, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.2969069646728344;

    uv_face = wrap_uv(off2, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.09447039785044732;

    uv_face = wrap_uv(-off2, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.09447039785044732;

    uv_face = wrap_uv(off3, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.010381362401148057;

    uv_face = wrap_uv(off3, face);
    moments += texture(shadow_maps[int(uv_face.z)], uv_face.xy).xy * 0.010381362401148057;

    return vec4(moments, 0.f, 0.f);
}

void main()
{
    frag_color_0 = blur9(0);
    frag_color_1 = blur9(1);
    frag_color_2 = blur9(2);
    frag_color_3 = blur9(3);
    frag_color_4 = blur9(4);
    frag_color_5 = blur9(5);
}
