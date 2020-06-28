#version 450

layout (location = 0) in vec2 in_Position;

struct rectangle_info
{
	vec4 PositionSize;
	vec4 FillColorRadius;
	vec4 BorderColorSize;
};

layout (std430, binding=1) buffer rect_infos
{
    rectangle_info[] u_RectangleInfos;
};

layout (location = 0) flat out uint InstanceID;
layout (location = 1) out vec2 OutPosition;

uniform mat4 u_View;
uniform mat4 u_Proj;

void main()
{
    vec2 Translation = u_RectangleInfos[gl_InstanceID].PositionSize.xy;
    Translation = mix(Translation - 100, Translation + 100, in_Position * 0.5 + 0.5);
    vec2 Scale = u_RectangleInfos[gl_InstanceID].PositionSize.zw;

    vec2 Position = in_Position * Scale + Translation;
    gl_Position = u_Proj * u_View * vec4(Position, 0, 1);

    OutPosition = vec2(u_View * vec4(Position, 0, 1));

    InstanceID = gl_InstanceID;
}
