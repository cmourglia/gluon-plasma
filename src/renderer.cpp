#include "renderer.h"
#include "render_backend.h"

#include <glad/glad.h>

#include <EASTL/numeric_limits.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <loguru.hpp>

#include <optick.h>

struct pos_texcoord_vertex
{
	glm::vec2 Position;
	// glm::vec2 Coords;
	// glm::vec4 Color;

	static void Init()
	{
		// sLayout.begin()
		// .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		// .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		// .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
		// .end();
	}
};

constexpr eastl::array<pos_texcoord_vertex, 4> k_QuadVertices = {
    pos_texcoord_vertex{glm::vec2{-1.0f, -1.0f} /*, glm::vec2{1.0f, 0.0f}*/},
    pos_texcoord_vertex{glm::vec2{-1.0f, 1.0f} /*, glm::vec2{0.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, 1.0f} /*, glm::vec2{1.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, -1.0f} /*, glm::vec2{1.0f, 0.0f}*/},
};

constexpr eastl::array<uint16_t, 6> k_QuadIndices = {0, 2, 1, 0, 3, 2};

static vertex_array_handle  g_VertexArrayHandle  = GLUON_INVALID_HANDLE;
static vertex_buffer_handle g_VertexBufferHandle = GLUON_INVALID_HANDLE;
static index_buffer_handle  g_IndexBufferHandle  = GLUON_INVALID_HANDLE;

struct rectangle
{
	glm::vec2 Position;
	glm::vec2 Size;
	color     FillColor;
	color     BorderColor;
	float     BorderSize;
};

struct rendering_context
{
	eastl::vector<glm::vec4> Positions;
	eastl::vector<glm::vec4> FillColorRadii;
	eastl::vector<glm::vec4> BorderColorSize;

	eastl::vector<rectangle> Rectangles;

	glm::mat4 ViewMatrix;
	glm::mat4 ProjMatrix;

	program_handle Program;
};

void DrawRectangle(rendering_context* Context,
                   float              X,
                   float              Y,
                   float              Width,
                   float              Height,
                   color              Color,
                   float              BorderWidth /*= 1.0f*/,
                   color              BorderColor /*= {0.0f, 0.0f, 0.0f, 1.0f}*/)
{
	OPTICK_EVENT();

	rectangle Rectangle;
	Rectangle.Position    = glm::vec2(X, Y);
	Rectangle.Size        = glm::vec2(Width, Height) / 2.0f;
	Rectangle.BorderColor = BorderColor;
	Rectangle.FillColor   = Color;

	Context->Rectangles.push_back(Rectangle);
	Context->Positions.push_back(glm::vec4(X, Y, Width, Height));
}

// template <typename data_t>
// inline void UpdateBuffer(bgfx::DynamicVertexBufferHandle Handle, const eastl::vector<data_t>& Data)
// {
// 	bgfx::update(Handle, 0, bgfx::copy(Data.data(), (uint32_t)Data.size() * sizeof(data_t)));
// }

void Flush(rendering_context* Context)
{
	OPTICK_EVENT();

	if (!IsValid(g_VertexBufferHandle))
	{
		pos_texcoord_vertex::Init();
		uint32_t VertexBufferSize = (uint32_t)k_QuadVertices.size() * sizeof(pos_texcoord_vertex);
		uint32_t IndexBufferSize  = (uint32_t)k_QuadIndices.size() * sizeof(uint16_t);

		g_VertexBufferHandle = CreateVertexBuffer(); // k_QuadVertices.data(), VertexBufferSize, pos_texcoord_vertex::sLayout);
		glNamedBufferStorage(g_VertexBufferHandle.Idx,
		                     k_QuadVertices.size() * sizeof(pos_texcoord_vertex),
		                     k_QuadVertices.data(),
		                     GL_DYNAMIC_STORAGE_BIT);

		g_IndexBufferHandle = CreateIndexBuffer(); // bgfx::makeRef(k_QuadIndices.data(), IndexBufferSize));
		glNamedBufferStorage(g_IndexBufferHandle.Idx,
		                     k_QuadIndices.size() * sizeof(uint16_t),
		                     k_QuadIndices.data(),
		                     GL_DYNAMIC_STORAGE_BIT);

		g_VertexArrayHandle = CreateVertexArray();
		glVertexArrayVertexBuffer(g_VertexArrayHandle.Idx, 0, g_VertexBufferHandle.Idx, 0, sizeof(pos_texcoord_vertex));
		glVertexArrayElementBuffer(g_VertexArrayHandle.Idx, g_IndexBufferHandle.Idx);

		glEnableVertexArrayAttrib(g_VertexArrayHandle.Idx, 0);
		// glEnableVertexArrayAttrib(g_VertexArrayHandle.Idx, 1);

		glVertexArrayAttribFormat(g_VertexArrayHandle.Idx, 0, 2, GL_FLOAT, GL_FALSE, offsetof(pos_texcoord_vertex, Position));
		// glVertexArrayAttribFormat(g_VertexArrayHandle.Idx, 1, 2, GL_FLOAT, GL_FALSE, offsetof(pos_texccord_vertex, Texcoord));

		glVertexArrayAttribBinding(g_VertexArrayHandle.Idx, 0, 0);
		// glVertexArrayAttribBinding(g_VertexArrayHandle.Idx, 1, 0);
	}

	glUseProgram(Context->Program.Idx);

	const GLint ViewLoc  = glGetUniformLocation(Context->Program.Idx, "u_View");
	const GLint ProjLoc  = glGetUniformLocation(Context->Program.Idx, "u_Proj");
	const GLint ModelLoc = glGetUniformLocation(Context->Program.Idx, "u_Model");
	const GLint ColorLoc = glGetUniformLocation(Context->Program.Idx, "u_Color");

	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(Context->ViewMatrix));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Context->ProjMatrix));

	for (auto Rectangle : Context->Rectangles)
	{
		glBindVertexArray(g_VertexArrayHandle.Idx);

		glm::mat4 Transform = glm::mat4(1.0f);
		Transform           = glm::translate(Transform, glm::vec3(Rectangle.Position, 0.0f));
		Transform           = glm::scale(Transform, glm::vec3(Rectangle.Size, 1.0f));

		glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Transform));

		glUniform4fv(ColorLoc, 1, &Rectangle.FillColor.r);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	}

	Context->Rectangles.clear();
	Context->Positions.clear();
}
