#version 450

layout (location = 0) flat in uint InstanceID;
layout (location = 1) in vec2 Position;
layout (location = 2) in vec2 Texcoord;
layout (location = 3) flat in uint TextureIndex;
layout (location = 4) in vec4 FillColor;

layout (location = 0) out vec4 out_Color;

uniform sampler2D u_Textures[16];

float Median(float r, float g, float b)
{
	return max(min(r, g), min(max(r, g), b));
}

const float width = 0.5;
const float border = 0.1;
const float borderWidth = 0.6;
const float borderEdge = 0.1;

void main()
{
	vec3 Sample = texture(u_Textures[TextureIndex], Texcoord).rgb;
	// vec3 DropShadowSample = texture(u_Textures[0], Texcoord + vec2(-0.0025, -0.0025)).rgb;

	// float Distance = 1.0 - Median(Sample.r, Sample.g, Sample.b);
	// float DropShadowDistance = 1.0 - Median(DropShadowSample.r, DropShadowSample.g, DropShadowSample.b);

	// float Alpha = 1 - smoothstep(width, width + border, Distance);
	// float OutlineAlpha = 1.0 - smoothstep(borderWidth, clamp(borderWidth + borderEdge, 0.0, 1.0), DropShadowDistance);
	// float OverallAlpha = Alpha + (1.0 - Alpha) * OutlineAlpha;

	// vec3 OverallColor = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 0.0), Alpha / OverallAlpha);

	// out_Color = vec4(OverallColor, OverallAlpha);

	float SignedDist = Median(Sample.r, Sample.g, Sample.b) - 0.5;

	ivec2 TextureSize = textureSize(u_Textures[0], 0);
	float dx = dFdx(Texcoord.x) * TextureSize.x;
	float dy = dFdy(Texcoord.y) * TextureSize.y;
	float ToPixels = 12 * inversesqrt(dx * dx + dy * dy);

	float Opacity = clamp(SignedDist * ToPixels + 0.5, 0.0, 1.0);
	out_Color = vec4(FillColor.rgb, Opacity);
	// out_Color = vec4(vec3(Opacity), 1.0);
	// out_Color = vec4(vec3(ToPixels), 1);
}
