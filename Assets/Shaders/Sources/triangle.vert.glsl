#version 450

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_Texcoord;

layout (location = 0) out vec3 out_Color;

void main() {
	vec3 Position = (in_Position * 0.01) + vec3(0, -0.5, 1);
	gl_Position = vec4(Position, 1.0);

	out_Color = in_Normal * 0.5 + 0.5;
	out_Color = vec3(in_Texcoord, 0);
}
