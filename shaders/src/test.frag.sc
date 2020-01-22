$input v_position, v_texcoord

#include <bgfx_shader.sh>

uniform vec4 u_color;

const vec4 Box = vec4(200, 250, 250, 200);

vec4 Error(vec4 x) {
	vec4 s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
	x *= x;
	return s - s / (x * x);
}

// Return the mask for the shadow of a box from Lower to Upper
float BoxShadow(vec2 Lower, vec2 Upper, vec2 Point, float Sigma) {
	vec4 query = vec4(Point - Lower, Upper - Point);
	vec4 integral = 0.5 + 0.5 * Error(query * (sqrt(0.5) / Sigma));
	return (integral.z - integral.x) * (integral.w - integral.y);
}

float GetDistance(vec2 Lower, vec2 Upper, vec2 Point)
{
	vec2 Center = (Lower + Upper) * 0.5;
	float x = Center.x, y = Center.y;
	float px = Point.x, py = Point.y;
	float w = Upper.x - Lower.x;
	float h = Upper.y - Lower.y;

	float dx = max(abs(px - x) - w * 0.5, 0);
	float dy = max(abs(py - y) - h * 0.5, 0);

	return dx * dx + dy * dy;
}

float GetAlpha(float d, float s)
{
	return (s - d) / s;
}

void main()
{
	// float Alpha = BoxShadow(vec2(200, 250), vec2(250, 200), vec2(gl_FragCoord.x, gl_FragCoord.y), 10);
	float Distance = GetDistance(vec2(200, 200), vec2(250, 250), vec2(gl_FragCoord.x, gl_FragCoord.y));

	if (Distance > 0)
	{
		float Alpha = GetAlpha(Distance, 100);
		gl_FragColor = vec4(0.2, 0.2, 0.2, 1) * Alpha;
	}
	else
	{
		gl_FragColor = vec4(1, 1, 1, 1);
	}

	// else if (gl_FragCoord.x > Box.z)
	// {
	// 	gl_FragColor = vec4(1, 0, 1, 1);
	// }
	// // vec4  = gl_FragCoord.xy;
	// if (length(dir) > 0) {
	// 	gl_FragColor = vec4(0, 0, 0, 1);
	// } else {
	// 	gl_FragColor = vec4(1, 0, 0, 1);
	// }
	// gl_FragColor = vec4(length(dir), 0, 0, 1);
	// float d = length(gl_FragCoord.xy - v_position);
	// gl_FragColor = u_color;
}
