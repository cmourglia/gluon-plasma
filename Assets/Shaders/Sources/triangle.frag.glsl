#version 450

layout (binding = 1) readonly buffer b_Color
{
	vec4 Color;
};

layout (location = 0) in vec3 in_Color;

layout (location = 0) out vec4 out_Color;

void main() {
	out_Color = vec4(in_Color, 1.0);
	out_Color = Color;
}
