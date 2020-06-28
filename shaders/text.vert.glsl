#version 450

layout (location = 0) in vec2 in_Position;

struct glyph_info
{
	vec4 PositionTranslate;
	vec4 Scale; // xy -> Scale, z -> GlobalScale
	vec4 Texcoords;
	vec4 FillColor;
	uint TextureIndex;
};

layout (std430, binding =1 ) buffer glyph_infos
{
	glyph_info[] u_GlyphInfos;
};

layout (location = 0) flat out uint InstanceID;
layout (location = 1) out vec2 OutPosition;
layout (location = 2) out vec2 OutTexcoord;
layout (location = 3) flat out uint TextureIndex;
layout (location = 4) out vec4 FillColor;

uniform mat4 u_View;
uniform mat4 u_Proj;
uniform vec2 u_ViewportSize;

void main()
{
	glyph_info GlyphInfos = u_GlyphInfos[gl_InstanceID];
	vec2 Scale = GlyphInfos.Scale.xy;
	float GlobalScale = GlyphInfos.Scale.z;

	vec2 WorldPosition = GlyphInfos.PositionTranslate.xy;
	vec2 Translate = GlyphInfos.PositionTranslate.zw;

	vec2 Position = in_Position + vec2(1, 0);
	Position = (Position * Scale + Translate) * GlobalScale + WorldPosition;
	Position.y = u_ViewportSize.y - Position.y;

	gl_Position = u_Proj * u_View * vec4(Position, 0, 1);

	int XIndex = int(in_Position.x + 1.5);
	int YIndex = int(in_Position.y + 1.5) + 1;
	vec4 InTexcoord = GlyphInfos.Texcoords;
	OutTexcoord = vec2(InTexcoord[XIndex], InTexcoord[YIndex]);

	InstanceID = gl_InstanceID;
	TextureIndex = GlyphInfos.TextureIndex;
	FillColor = GlyphInfos.FillColor;
}
