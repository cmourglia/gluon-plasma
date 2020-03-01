#version 450

layout (location = 0) flat in uint InstanceID;

layout (location = 0) out vec4 out_Color;

struct rectangle_info
{
    vec4 PositionSize;
    vec4 FillColor;
    vec4 BorderColorSize;
};

layout (std430, binding=1) buffer rect_infos
{
    rectangle_info[] u_RectangleInfos;
};

uniform vec4 u_Color;

void main()
{
    out_Color = u_RectangleInfos[InstanceID].FillColor;
}
