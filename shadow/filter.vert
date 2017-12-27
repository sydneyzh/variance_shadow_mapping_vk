#version 450 core

layout (location = 0) out vec2 uv_out;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec2 uv = vec2( gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2 );
    gl_Position = vec4(uv* 2.0f - 1.0f, 0.0f, 1.0f);
    uv_out = uv;
}
