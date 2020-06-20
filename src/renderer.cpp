#include "renderer.h"
#include "render_backend.h"
#include "gln_math.h"

#include <glad/glad.h>

#include <EASTL/numeric_limits.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>

#include <thread>
#include <mutex>
#include <chrono>
#include <filesystem>
#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <loguru.hpp>

#include <optick.h>

namespace gln
{

struct pos_texcoord_vertex
{
	vec2 Position;
	// vec2 Texcoords;
	// vec4 Color;

	static vertex_layout s_Layout;

	static void Init() { s_Layout.Begin().Add(DataType_Float, 2).End(); }
};

vertex_layout pos_texcoord_vertex::s_Layout = vertex_layout();

constexpr eastl::array<pos_texcoord_vertex, 4> k_QuadVertices = {
    pos_texcoord_vertex{vec2{-1.0f, -1.0f} /*, vec2{1.0f, 0.0f}*/},
    pos_texcoord_vertex{vec2{-1.0f, 1.0f} /*, vec2{0.0f, 1.0f}*/},
    pos_texcoord_vertex{vec2{1.0f, 1.0f} /*, vec2{1.0f, 1.0f}*/},
    pos_texcoord_vertex{vec2{1.0f, -1.0f} /*, vec2{1.0f, 0.0f}*/},
};

constexpr eastl::array<uint16_t, 6> k_QuadIndices = {0, 2, 1, 0, 3, 2};

#pragma pack(push, 1)
struct rectangle
{
	vec2  Position;
	vec2  Size;
	color FillColorRadius;
	color BorderColorSize;
};
#pragma pack(pop)

struct rendering_context
{
	eastl::vector<vec4> Positions;
	eastl::vector<vec4> FillColorRadii;
	eastl::vector<vec4> BorderColorSize;

	eastl::vector<rectangle> Rectangles;
	size_t                   LastRectangleCount;

	float ViewMatrix[16];
	float ProjMatrix[16];

	program_handle Program = GLUON_INVALID_HANDLE;

	vertex_array_handle  VertexArray  = GLUON_INVALID_HANDLE;
	index_buffer_handle  IndexBuffer  = GLUON_INVALID_HANDLE;
	vertex_buffer_handle VertexBuffer = GLUON_INVALID_HANDLE;

	float ViewportWidth, ViewportHeight;

	buffer_handle RectangleInfoSSBO;
	void*         RectangleInfoSSBOPtr = nullptr;
};

struct draw_command
{
	GLuint VertexCount;
	GLuint InstanceCount;
	GLuint FirstIndex;
	GLuint BaseVertex;
	GLuint BaseInstance;
};

namespace fs = std::filesystem;

static fs::file_time_type g_VertexShaderTime;
static fs::file_time_type g_FragmentShaderTime;

rendering_context* CreateRenderingContext()
{
	if (!gladLoadGL())
	{
		LOG_F(FATAL, "Cannot load OpenGL functions");
	}

	LOG_F(INFO, "OpenGL:\n\tVersion %s\n\tVendor %s", glGetString(GL_VERSION), glGetString(GL_VENDOR));

#ifdef _DEBUG
	EnableDebugging();
#endif

	rendering_context* Context = new rendering_context();

	g_VertexShaderTime   = fs::last_write_time(fs::path("shaders/test.vert.glsl"));
	g_FragmentShaderTime = fs::last_write_time(fs::path("shaders/test.frag.glsl"));

	auto VertexShaderHandle   = LoadShader("shaders/test.vert.glsl", ShaderType_Vertex);
	auto FragmentShaderHandle = LoadShader("shaders/test.frag.glsl", ShaderType_Fragment);
	auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

	Context->Program = ProgramHandle;

	pos_texcoord_vertex::Init();
	uint32_t VertexBufferSize = (uint32_t)k_QuadVertices.size() * sizeof(pos_texcoord_vertex);
	uint32_t IndexBufferSize  = (uint32_t)k_QuadIndices.size() * sizeof(uint16_t);

	Context->VertexBuffer = CreateVertexBuffer(k_QuadVertices.size() * sizeof(pos_texcoord_vertex), k_QuadVertices.data());
	Context->IndexBuffer  = CreateIndexBuffer(k_QuadIndices.size() * sizeof(uint16_t), k_QuadIndices.data());

	Context->VertexArray = CreateVertexArray(Context->IndexBuffer);
	AttachVertexBuffer(Context->VertexArray, Context->VertexBuffer, pos_texcoord_vertex::s_Layout);

	Context->LastRectangleCount   = 128 * 128;
	Context->RectangleInfoSSBO    = CreateImmutableBuffer(128 * 128 * sizeof(rectangle));
	Context->RectangleInfoSSBOPtr = MapBuffer(Context->RectangleInfoSSBO);

	return Context;
}

void DestroyRenderingContext(rendering_context* Context)
{
	if (glIsProgram(Context->Program.Idx))
	{
		glDeleteProgram(Context->Program.Idx);
	}
	delete Context;
}

void Resize(rendering_context* Context, float Width, float Height)
{
	Context->ViewportWidth  = Width;
	Context->ViewportHeight = Height;
}

void SetCameraInfo(rendering_context* Context, const float* ViewMatrix, const float* ProjMatrix)
{
	memcpy(Context->ViewMatrix, ViewMatrix, 16 * sizeof(float));
	memcpy(Context->ProjMatrix, ProjMatrix, 16 * sizeof(float));
}

void DrawRectangle(rendering_context* Context,
                   float              X,
                   float              Y,
                   float              Width,
                   float              Height,
                   color              Color,
                   float              Radius /* = 0.0f */,
                   float              BorderWidth /*= 1.0f*/,
                   color              BorderColor /*= {0.0f, 0.0f, 0.0f, 1.0f}*/
)
{
	OPTICK_EVENT();

	rectangle Rectangle;
	Rectangle.Position          = vec2(X, Y);
	Rectangle.Size              = vec2(Width, Height) / 2.0f;
	Rectangle.BorderColorSize   = BorderColor;
	Rectangle.BorderColorSize.A = BorderWidth;
	Rectangle.FillColorRadius   = Color;
	Rectangle.FillColorRadius.A = Clamp(Radius, 0.0, Min(Width, Height) / 2.0f);

	Context->Rectangles.push_back(Rectangle);
	Context->Positions.push_back(vec4(X, Y, Width, Height));
}

void Flush(rendering_context* Context)
{
	OPTICK_EVENT();

	auto VertexTime   = fs::last_write_time(fs::path("shaders/test.vert.glsl"));
	auto FragmentTime = fs::last_write_time(fs::path("shaders/test.frag.glsl"));

	if (VertexTime > g_VertexShaderTime || FragmentTime > g_FragmentShaderTime)
	{
		g_VertexShaderTime   = VertexTime;
		g_FragmentShaderTime = FragmentTime;

		auto VertexShaderHandle   = LoadShader("shaders/test.vert.glsl", ShaderType_Vertex);
		auto FragmentShaderHandle = LoadShader("shaders/test.frag.glsl", ShaderType_Fragment);
		auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

		Context->Program = ProgramHandle;
	}

	glClearColor(0.2f, 0.4f, 0.5f, 1.0f);
	glViewport(0, 0, (GLsizei)Context->ViewportWidth, (GLsizei)Context->ViewportHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(Context->Program.Idx);

	const GLint ViewLoc         = glGetUniformLocation(Context->Program.Idx, "u_View");
	const GLint ProjLoc         = glGetUniformLocation(Context->Program.Idx, "u_Proj");
	const GLint ViewportSizeLoc = glGetUniformLocation(Context->Program.Idx, "u_ViewportSize");

	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, Context->ViewMatrix);
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, Context->ProjMatrix);
	glUniform2f(ViewportSizeLoc, Context->ViewportWidth, Context->ViewportHeight);

	const int32_t RectangleCount = (int32_t)Context->Rectangles.size();

	if (Context->LastRectangleCount < RectangleCount)
	{
		UnmapBuffer(Context->RectangleInfoSSBO);
		ResizeImmutableBuffer(&Context->RectangleInfoSSBO, RectangleCount * sizeof(rectangle));

		Context->LastRectangleCount   = RectangleCount;
		Context->RectangleInfoSSBOPtr = MapBuffer(Context->RectangleInfoSSBO);
	}

	memcpy(Context->RectangleInfoSSBOPtr, Context->Rectangles.data(), RectangleCount * sizeof(rectangle));

	glBindVertexArray(Context->VertexArray.Idx);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Context->RectangleInfoSSBO.Idx);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, RectangleCount);

	glDisable(GL_BLEND);

	Context->Rectangles.clear();
	Context->Positions.clear();
}
}
