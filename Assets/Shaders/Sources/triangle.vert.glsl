#version 450

struct GVertex
{
	float PX, PY, PZ;
	float NX, NY, NZ;
	float TU, TV;
};

layout (binding = 0) readonly buffer VertexBuffer
{
	GVertex Vertices[];
};

layout(binding = 1) uniform ObjectBuffer {
    mat4 Model;
    mat4 View;
    mat4 Proj;
} Transforms;

layout (location = 0) out vec3 out_Color;

void main() {
	gl_Position = Transforms.Proj * Transforms.View * Transforms.Model * vec4(Vertices[gl_VertexIndex].PX, Vertices[gl_VertexIndex].PY, Vertices[gl_VertexIndex].PZ, 1.0);
	out_Color = vec3(Vertices[gl_VertexIndex].NX, Vertices[gl_VertexIndex].NY, Vertices[gl_VertexIndex].NZ) * 0.5 + 0.5;
	// out_Color = vec3(Texcoord, 0);
}
