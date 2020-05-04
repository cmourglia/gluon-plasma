#version 450

struct GVertex
{
	float PX, PY, PZ;
	float NX, NY, NZ;
	float TU, TV;
};

layout (binding = 0) readonly buffer b_Vertices
{
	GVertex Vertices[];
};

layout (location = 0) out vec3 out_Color;

void main() {
	GVertex Vertex = Vertices[gl_VertexIndex];

	vec3 Position = vec3(Vertex.PX, Vertex.PY, Vertex.PZ);
	vec3 Normal = vec3(Vertex.NX, Vertex.NY, Vertex.NZ);
	vec2 Texcoord = vec2(Vertex.TU, Vertex.TV);

	Position = (Position * 0.01) + vec3(0, -0.5, 1);

	gl_Position = vec4(Position, 1.0);

	out_Color = Normal * 0.5 + 0.5;
	out_Color = vec3(Texcoord, 0);
}
