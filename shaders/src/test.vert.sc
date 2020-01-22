$input a_position //, a_texcoord0
$output v_texcoord, v_position
// $output vec2 v_position;

#include <bgfx_shader.sh>

void main()
{
    vec2 position = mix(vec2(-0.5, -0.5), vec2(1.5, 1.5), a_position);
    gl_Position = mul(u_modelViewProj, vec4(position, 0.0, 1.0));
    // vertex = gl_Position
    // gl_Position = vec4(a_position * 2 - 1, 0, 1);

    v_position = mul(u_model[0], vec4(position, 0.0, 1.0));
    v_texcoord = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0));
    // v_texcoord = a_position.xy;
}
