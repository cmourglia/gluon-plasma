#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

#include "uniforms.sh"

struct circle_t
{
	vec3 FillColor;
	vec3 BorderColor;
	float Radius;
	vec2 Position;
};

BUFFER_RO(CirclePositions, vec4, 0);
BUFFER_RO(CircleFillColorRadius, vec4, 1);
BUFFER_RO(CircleBorderColorSize, vec4, 2);

float sdLine(in vec2 p, in vec2 a, in vec2 b)
{
	vec2  pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba * h);
}

vec3 Line(in vec3 buf, in vec2 a, in vec2 b, in vec2 p, in vec2 w, in vec4 col)
{
	float f = sdLine(p, a, b);
	float g = fwidth(f) * w.y;
	return mix(buf, col.xyz, col.w * (1.0 - smoothstep(w.x - g, w.x + g, f)));
}

vec4 DrawCircle(vec3 InColor, circle_t Circle, vec2 Position)
{
	float Distance = length(Position - Circle.Position);
	vec3 OutColor = InColor;
	OutColor = mix(OutColor, Circle.FillColor, step(Distance, Circle.Radius));
	// OutColor = Distance;
	// OutColor = mix(OutColor, Circle.BorderColor, 1.0 - smoothstep(0.01, 0.002, abs(Distance - Circle.Radius)));
	return vec4(OutColor, 1 - Distance);
}

vec2 ToScreenSpace(vec2 Coords, vec2 ScreenSize)
{
	return (2.0 * Coords - ScreenSize) / min(ScreenSize.x, ScreenSize.y);
}

void main()
{
	// circle_t Circles[4];
	// Circles[0].Position = vec2(-0.5, -0.5);
	// Circles[0].Radius = 0.75;
	// Circles[0].InnerColor = vec3(118, 110, 200) / 255;

	// Circles[1].Position = vec2(0.5, -0.5);
	// Circles[1].Radius = 0.75;
	// Circles[1].InnerColor = vec3(255, 53, 94) / 255;

	// Circles[2].Position = vec2(0.5, 0.5);
	// Circles[2].Radius = 0.75;
	// Circles[2].InnerColor = vec3(50, 105, 74) / 255;

	// Circles[3].Position = vec2(-0.5, 0.5);
	// Circles[3].Radius = 0.75;
	// Circles[3].InnerColor = vec3(138, 243, 163) / 255;

	vec2 uv = ToScreenSpace(gl_FragCoord.xy, uScreenSize);

	float p = length(uv);
	float r = 0.75;

	// background
	// vec3 col = vec3(0.5, 0.6, 0.7) + 0.2 * uv.y;
	vec4 col = 0.0;

	// for (int i = 0; i < ElementCount; ++i)
	// {
		circle_t Circle;
		Circle.Position = ToScreenSpace(uPosition, uScreenSize);
		Circle.Radius = uSize.x / uScreenSize.y;
		Circle.FillColor = uColor;
		Circle.BorderColor = 0.0;

		col = DrawCircle(col, Circle, uv);
	// }

	// col = DrawCircle(col, Circles[0], uv);
	// col = DrawCircle(col, Circles[1], uv);
	// col = DrawCircle(col, Circles[2], uv);
	// col = DrawCircle(col, Circles[3], uv);

	// gl_FragColor = h;
	// gl_FragColor = vec4(col, 1.0);
	// gl_FragColor = col;
	gl_FragColor = vec4(uColor.rgb, 1.0);
	gl_FragColor = vec4(1, 0, 0, 1);
}
