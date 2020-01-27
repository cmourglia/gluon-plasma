$input a_position, i_data0

#include <bgfx_shader.sh>

void main()
{
    vec2 Position = i_data0.xy;
    vec2 Size = i_data0.zw;

    vec2 WorldPos = Position + a_position * (Size * 0.5);
    gl_Position = mul(u_viewProj, vec4(WorldPos, 0, 1));
    // gl_Position = vec4(WorldPos, 0, 1);
    // gl_Position = vec4(a_position, 0, 1);


    // vec2 position = mix(vec2(-0.5, -0.5), vec2(1.5, 1.5), a_position);
    // gl_Position = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0));
    // vertex = gl_Position
    // gl_Position = vec4(a_position * 2 - 1, 0, 1);

    // v_position = mul(u_model[0], vec4(position, 0.0, 1.0));
    // v_texcoord = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0));
    // v_texcoord = a_position.xy;
    // gl_Position = vec4(a_position, 0.0f, 1.0f);
}
