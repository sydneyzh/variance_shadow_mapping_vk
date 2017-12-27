#version 450 core
#define SHADOW_MAP_SIZE 512.f
#define LIGHT_SIZE 20.f
#define BIAS 0.15f

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

layout(set = 1, binding = 1) uniform samplerCube shadow_map;

layout(location = 0) in vec4 world_pos_in;
layout(location = 1) in vec3 world_normal_in;
layout(location = 0) out vec4 frag_color;

float upper_bound_shadow(vec2 moments, float scene_depth)
{
    float p = step(scene_depth, moments.x + BIAS); // eliminates cubemap boundary thin line
    // 0 if moments.x < scene_depth; 1 if otherwise

    float variance = max(moments.y - moments.x * moments.x, 0.0001);
    // depth^2 - mean^2
    // ensure it as a denominator is not zero

    float d = scene_depth - moments.x;

    float p_max = variance / (variance + d * d);
    // from the theorem

    return max(p, p_max);
}

float sample_shadow(samplerCube shadow_map, vec3 l, float scene_depth)
{
    vec2 moments = texture(shadow_map, l).xy;
    // moments.x is mean, moments.y is depth^2

    return upper_bound_shadow(moments, scene_depth);
}

void main() {
    frag_color = vec4(0.f, 0.f, 0.f, 1.f);
    vec3 l = world_pos_in.xyz - plight_info_in.pos;
    float d = length(l);
    l = normalize(l);
    if (d < plight_info_in.range) {
	float lambertian = max(0.f, dot(world_normal_in, -l));
	float atten = max(0.f, min(1.f, d/plight_info_in.range));
	atten = 1.f - atten * atten;
	float shadow = sample_shadow(shadow_map, l, d);
	frag_color.xyz += vec3(atten * lambertian * shadow);
    }
}
