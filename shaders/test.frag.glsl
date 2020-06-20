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
}

void main()
{
	vec2 Position = InPosition;
	vec2 Center = vec2(u_View * vec4(u_RectangleInfos[InstanceID].PositionSize.xy, 0, 1));
	vec2 Size = u_RectangleInfos[InstanceID].PositionSize.zw;
	float Radius = Size.x / 3 ;

	float Border = 5;
	float BorderAA = 1;
	float Alpha = GetRectangleAlpha(Position, Center, Size, Radius);
	float AlphaBorder = GetRectangleAlpha(Position, Center, Size + Border, Radius == 0 ? 0 : Radius + Border);

	out_Color.a = 1.0;

	vec3 FillColor = u_RectangleInfos[InstanceID].FillColor.rgb;
	vec3 BorderColor = vec3(0);

	vec3 Color;
	if (Alpha < 0) {
		Color = FillColor;
	} else if (Alpha < BorderAA) {
		if (Border == 0) {
			out_Color.a = 1 - (Alpha / BorderAA);
			Color = FillColor;
		} else {
			Color = mix(FillColor, BorderColor, smoothstep(0.0, 1.0, Alpha / BorderAA));
		}
	} else if (AlphaBorder < 0) {
		// float a = smoothstep(0.0, 1250.0, Alpha * Size.x);
		// Color = mix(vec3(1, 0, 0), vec3(0, 1, 0), a);
		Color = BorderColor;
	} else if (AlphaBorder < BorderAA) {
		Color = BorderColor;
		out_Color.a = 1 - (AlphaBorder / BorderAA);
	} else {
		discard;
	}

	out_Color.rgb = Color;
}
