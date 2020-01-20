$input a_position //, a_texcoord0
// $output vec2 v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    mat4 xform = mul(u_proj, u_model[0]);
    gl_Position = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0));
    // v_texcoord0 = a_texcoord0;
}
