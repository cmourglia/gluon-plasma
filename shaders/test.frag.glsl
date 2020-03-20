#version 450

layout (location = 0) flat in uint InstanceID;
layout (location = 1) in vec2 InPosition;

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

uniform vec2 u_ViewportSize;
uniform mat4 u_View;

float GetRectangleAlpha(vec2 Position, vec2 Center, vec2 Size, float Radius)
{
    vec2 Q = abs(Position - Center) - Size + Radius;
    return (min(max(Q.x, Q.y), 0.0) + length(max(Q, 0.0)) - Radius);

    float Distance = length(Position - Center);

    return 1 - step(Radius, Distance);
}

void main()
{
    // vec2 Position = (2.0 * gl_FragCoord.xy / u_ViewportSize) / u_ViewportSize.y;
    vec2 Position = InPosition;
    vec2 Center = vec2(u_View * vec4(u_RectangleInfos[InstanceID].PositionSize.xy, 0, 1));
    // vec2 Center = (2.0 * u_RectangleInfos[InstanceID].PositionSize.xy / u_ViewportSize) / u_ViewportSize.y;
    // Center.y = 1 - Center.y;

    // vec2 Size = (2.0 * u_RectangleInfos[InstanceID].PositionSize.zw / u_ViewportSize) / u_ViewportSize.y;
    vec2 Size = u_RectangleInfos[InstanceID].PositionSize.zw;
    float Radius = Size.x;

    float Alpha = GetRectangleAlpha(Position, Center, Size, Radius) / length(Size);
    vec3 Color = vec3(1.0) - sign(Alpha) * vec3(0.1, 0.4, 0.7);
    Color *= 1.0 - exp(-3.0 * abs(Alpha));
    Color *= 0.8 + 0.2 * cos(150 * Alpha);
    Color = mix(Color, vec3(1.0, 0.0, 0.0), 1.0 - smoothstep(0.0, 0.05, abs(Alpha)));

    float Sign = sign(Alpha);

    out_Color = u_RectangleInfos[InstanceID].FillColor * Alpha;
    out_Color.rgb = Color;
    // out_Color = vec4(Alpha);
    // out_Color = vec4(vec3(Alpha), 1);
    // out_Color.a = Sign;
    // out_Color = vec4(Position, 0, 1);
    // out_Color = vec4(Center, 0, 1);
    // out_Color = vec4(0.5, 0.5, 0, 1);
    out_Color.a = 1;
}
