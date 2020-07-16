#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_renderer_p.h>
#include <gluon/api/gln_text.h>

#include <gluon/render_backend/gln_renderbackend.h>

#include <gluon/core/gln_math.h>

#include <glad/glad.h>

#include <EASTL/numeric_limits.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
#include <EASTL/string.h>
#include <EASTL/string_hash_map.h>

#include <thread>
#include <mutex>
#include <chrono>
#include <filesystem>
#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <loguru.hpp>

namespace gluon
{
struct pos_vertex
{
	vec2 Position;

	static vertex_layout s_Layout;

	static void Init() { s_Layout.Begin().Add(DataType_Float, 2).End(); }
};

vertex_layout pos_vertex::s_Layout = vertex_layout();

struct pos_texcoord_vertex
{
	vec2 Position;
	vec2 Texcoords;

	static vertex_layout s_Layout;

	static void Init() { s_Layout.Begin().Add(DataType_Float, 2).Add(DataType_Float, 2).End(); }
};

vertex_layout pos_texcoord_vertex::s_Layout = vertex_layout();

constexpr eastl::array<pos_vertex, 4> k_QuadVertices = {
    pos_vertex{vec2{-1.0f, -1.0f} /*, vec2{1.0f, 0.0f}*/},
    pos_vertex{vec2{-1.0f, 1.0f} /*, vec2{0.0f, 1.0f}*/},
    pos_vertex{vec2{1.0f, 1.0f} /*, vec2{1.0f, 1.0f}*/},
    pos_vertex{vec2{1.0f, -1.0f} /*, vec2{1.0f, 0.0f}*/},
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

#pragma pack(push, 1)
struct glyph_data
{
	vec2  Position;  // "World" position
	vec2  Translate; // Internal translation (position w.r.t to glyph's (0, 0))
	vec2  Scale;     // Rectangle size
	f32   GlobalScale;
	f32   Padding;
	vec4  Texcoords; // tx = int(x + 1.5), ty = int(y + 1.5) + 1
	color FillColor;
	u32   TextureIndex;
	vec3  Padding2;
};
#pragma pack(pop)

struct rendering_context
{
	f32 ViewMatrix[16];
	f32 ProjMatrix[16];
	f32 ViewportWidth, ViewportHeight;

	// Rectangle rendering
	eastl::vector<rectangle> Rectangles;
	size_t                   LastRectangleCount;

	program_handle RectProgram = GLUON_INVALID_HANDLE;

	vertex_array_handle RectVertexArray  = GLUON_INVALID_HANDLE;
	buffer_handle       RectIndexBuffer  = GLUON_INVALID_HANDLE;
	buffer_handle       RectVertexBuffer = GLUON_INVALID_HANDLE;

	buffer_handle RectangleInfoSSBO;
	void*         RectangleInfoSSBOPtr = nullptr;

	// Text rendering
	const char* CurrentFont;

	f32 TextScaleX, TextScaleY;

	eastl::string_hash_map<u32>   FontLookupMap;
	eastl::vector<font_atlas>     Fonts;
	eastl::vector<texture_handle> FontTextures;

	program_handle TextProgram = GLUON_INVALID_HANDLE;

	eastl::vector<glyph_data> GlyphData;
	size_t                    LastGlyphCount;

	buffer_handle TextInfoSSBO;
	void*         TextInfoSSBOPtr = nullptr;
};

static rendering_context* g_Context = nullptr;

namespace fs = std::filesystem;

static fs::file_time_type g_RectVShaderTime;
static fs::file_time_type g_RectFShaderTime;
static fs::file_time_type g_TextVShaderTime;
static fs::file_time_type g_TextFShaderTime;

namespace priv
{
	void CreateRenderingContext()
	{
		if (g_Context != nullptr)
		{
			return;
		}

		InitializeBackend();
#ifdef _DEBUG
		EnableDebugging();
#endif

		g_Context = new rendering_context();

		{
			g_RectVShaderTime = fs::last_write_time(fs::path("shaders/rect.vert.glsl"));
			g_RectFShaderTime = fs::last_write_time(fs::path("shaders/rect.frag.glsl"));

			auto VertexShaderHandle   = CreateShaderFromFile("shaders/rect.vert.glsl", ShaderType_Vertex);
			auto FragmentShaderHandle = CreateShaderFromFile("shaders/rect.frag.glsl", ShaderType_Fragment);
			auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

			g_Context->RectProgram = ProgramHandle;

			pos_vertex::Init();
			u32 VertexBufferSize = (u32)k_QuadVertices.size() * sizeof(pos_vertex);
			u32 IndexBufferSize  = (u32)k_QuadIndices.size() * sizeof(uint16_t);

			g_Context->RectVertexBuffer = CreateImmutableBuffer(k_QuadVertices.size() * sizeof(pos_vertex), k_QuadVertices.data());
			g_Context->RectIndexBuffer  = CreateImmutableBuffer(k_QuadIndices.size() * sizeof(uint16_t), k_QuadIndices.data());

			g_Context->RectVertexArray = CreateVertexArray(g_Context->RectIndexBuffer);
			AttachVertexBuffer(g_Context->RectVertexArray, g_Context->RectVertexBuffer, pos_vertex::s_Layout);

			g_Context->LastRectangleCount   = 128 * 128;
			g_Context->RectangleInfoSSBO    = CreateImmutableBuffer(128 * 128 * sizeof(rectangle));
			g_Context->RectangleInfoSSBOPtr = MapBuffer(g_Context->RectangleInfoSSBO);
		}

		{
			g_TextVShaderTime = fs::last_write_time(fs::path("shaders/text.vert.glsl"));
			g_TextFShaderTime = fs::last_write_time(fs::path("shaders/text.frag.glsl"));

			auto VertexShaderHandle   = CreateShaderFromFile("shaders/text.vert.glsl", ShaderType_Vertex);
			auto FragmentShaderHandle = CreateShaderFromFile("shaders/text.frag.glsl", ShaderType_Fragment);
			auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

			g_Context->TextProgram = ProgramHandle;

			g_Context->LastGlyphCount = 0;
			g_Context->TextInfoSSBO   = CreateImmutableBuffer(0);
		}
	}

	void DestroyRenderingContext()
	{
		if (glIsProgram(g_Context->RectProgram.Idx))
		{
			glDeleteProgram(g_Context->RectProgram.Idx);
		}
		delete g_Context;
		g_Context = nullptr;
	}

	void Resize(f32 Width, f32 Height)
	{
		g_Context->ViewportWidth  = Width;
		g_Context->ViewportHeight = Height;

		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (f32)Width, (f32)Height, 0.0f, 0.0f, 100.0f);

		memcpy(g_Context->ViewMatrix, glm::value_ptr(ViewMatrix), 16 * sizeof(f32));
		memcpy(g_Context->ProjMatrix, glm::value_ptr(ProjMatrix), 16 * sizeof(f32));
	}

	void SetTextScale(f32 ScaleX, f32 ScaleY)
	{
		g_Context->TextScaleX = ScaleX;
		g_Context->TextScaleY = ScaleY;
	}

	void RenderRectangles()
	{
		const i32 RectangleCount = (i32)g_Context->Rectangles.size();

		if (RectangleCount == 0)
		{
			return;
		}

		auto VertexTime   = fs::last_write_time(fs::path("shaders/rect.vert.glsl"));
		auto FragmentTime = fs::last_write_time(fs::path("shaders/rect.frag.glsl"));

		if (VertexTime > g_RectVShaderTime || FragmentTime > g_RectFShaderTime)
		{
			g_RectVShaderTime = VertexTime;
			g_RectFShaderTime = FragmentTime;

			auto VertexShaderHandle   = CreateShaderFromFile("shaders/rect.vert.glsl", ShaderType_Vertex);
			auto FragmentShaderHandle = CreateShaderFromFile("shaders/rect.frag.glsl", ShaderType_Fragment);
			auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

			g_Context->RectProgram = ProgramHandle;
		}

		glUseProgram(g_Context->RectProgram.Idx);

		const i32 ViewLoc         = glGetUniformLocation(g_Context->RectProgram.Idx, "u_View");
		const i32 ProjLoc         = glGetUniformLocation(g_Context->RectProgram.Idx, "u_Proj");
		const i32 ViewportSizeLoc = glGetUniformLocation(g_Context->RectProgram.Idx, "u_ViewportSize");

		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, g_Context->ViewMatrix);
		glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, g_Context->ProjMatrix);
		glUniform2f(ViewportSizeLoc, g_Context->ViewportWidth, g_Context->ViewportHeight);

		if (g_Context->LastRectangleCount < RectangleCount)
		{
			UnmapBuffer(g_Context->RectangleInfoSSBO);
			ResizeImmutableBuffer(&g_Context->RectangleInfoSSBO, RectangleCount * sizeof(rectangle));

			g_Context->LastRectangleCount   = RectangleCount;
			g_Context->RectangleInfoSSBOPtr = MapBuffer(g_Context->RectangleInfoSSBO);
		}

		memcpy(g_Context->RectangleInfoSSBOPtr, g_Context->Rectangles.data(), RectangleCount * sizeof(rectangle));

		glBindVertexArray(g_Context->RectVertexArray.Idx);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_Context->RectangleInfoSSBO.Idx);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, RectangleCount);

		g_Context->Rectangles.clear();
	}

	void RenderTexts()
	{
		const i32 GlyphCount = (i32)g_Context->GlyphData.size();

		auto VertexTime   = fs::last_write_time(fs::path("shaders/text.vert.glsl"));
		auto FragmentTime = fs::last_write_time(fs::path("shaders/text.frag.glsl"));

		if (VertexTime > g_TextVShaderTime || FragmentTime > g_TextFShaderTime)
		{
			g_TextVShaderTime = VertexTime;
			g_TextFShaderTime = FragmentTime;

			auto VertexShaderHandle   = CreateShaderFromFile("shaders/text.vert.glsl", ShaderType_Vertex);
			auto FragmentShaderHandle = CreateShaderFromFile("shaders/text.frag.glsl", ShaderType_Fragment);
			auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

			g_Context->TextProgram = ProgramHandle;
		}

		glUseProgram(g_Context->TextProgram.Idx);

		const i32 ViewLoc         = glGetUniformLocation(g_Context->TextProgram.Idx, "u_View");
		const i32 ProjLoc         = glGetUniformLocation(g_Context->TextProgram.Idx, "u_Proj");
		const i32 ViewportSizeLoc = glGetUniformLocation(g_Context->TextProgram.Idx, "u_ViewportSize");

		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, g_Context->ViewMatrix);
		glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, g_Context->ProjMatrix);
		glUniform2f(ViewportSizeLoc, g_Context->ViewportWidth, g_Context->ViewportHeight);

		if (g_Context->LastGlyphCount < GlyphCount)
		{
			UnmapBuffer(g_Context->TextInfoSSBO);
			ResizeImmutableBuffer(&g_Context->TextInfoSSBO, GlyphCount * sizeof(glyph_data));

			g_Context->LastGlyphCount  = GlyphCount;
			g_Context->TextInfoSSBOPtr = MapBuffer(g_Context->TextInfoSSBO);
		}

		memcpy(g_Context->TextInfoSSBOPtr, g_Context->GlyphData.data(), GlyphCount * sizeof(glyph_data));

		glBindVertexArray(g_Context->RectVertexArray.Idx);

		u32 CurrentIndex = 0;
		for (const auto& Font : g_Context->FontTextures)
		{
			glBindTextureUnit(CurrentIndex++, Font.Idx);
		}

		i32 TextureBindings[] = {0, 1, 2, 3, 4, 5, 6, 7};

		const i32 TexturesLoc = glGetUniformLocation(g_Context->TextProgram.Idx, "u_Textures");
		glUniform1iv(TexturesLoc, 8, TextureBindings);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_Context->TextInfoSSBO.Idx);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, GlyphCount);

		g_Context->GlyphData.clear();
	}

	void Flush()
	{
		glClearColor(0.2f, 0.4f, 0.5f, 1.0f);
		glViewport(0, 0, (GLsizei)g_Context->ViewportWidth, (GLsizei)g_Context->ViewportHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RenderRectangles();
		RenderTexts();

		glDisable(GL_BLEND);
	}
}

void DrawRectangle(f32   X,
                   f32   Y,
                   f32   Width,
                   f32   Height,
                   color Color,
                   f32   Radius /* = 0.0f */,
                   f32   BorderWidth /*= 1.0f*/,
                   color BorderColor /*= {0.0f, 0.0f, 0.0f, 1.0f}*/
)
{
	rectangle Rectangle;
	Rectangle.Position          = vec2(X, Y);
	Rectangle.Size              = vec2(Width, Height) / 2.0f;
	Rectangle.BorderColorSize   = BorderColor;
	Rectangle.BorderColorSize.A = BorderWidth;
	Rectangle.FillColorRadius   = Color;
	Rectangle.FillColorRadius.A = Clamp(Radius, 0.0, Min(Width, Height) / 2.0f);

	g_Context->Rectangles.push_back(Rectangle);
}

void SetFont(const char* FontName)
{
	g_Context->CurrentFont = FontName;

	// Load FontData if needed
	if (g_Context->FontLookupMap.find(FontName) == g_Context->FontLookupMap.end())
	{
		auto Atlas = LoadFontAtlas(FontName);

		// FIXME: We probably do not want to upload textures at this point
		texture_handle Texture = CreateTexture(Atlas.Width, Atlas.Height, 3);
		SetTextureWrapping(Texture, WrapMode_ClampToBorder, WrapMode_ClampToBorder);
		SetTextureData(Texture, Atlas.Data);

		g_Context->FontLookupMap[FontName] = (u32)g_Context->Fonts.size();
		g_Context->FontTextures.push_back(Texture);
		g_Context->Fonts.push_back(std::move(Atlas));
	}
}

void DrawText(const char32_t* Text, f32 PixelSize, f32 X, f32 Y, color FillColor)
{
	const char32_t* Char = Text;

	f32 CursorX = X;
	f32 CursorY = Y;

	const u32 FontIndex = g_Context->FontLookupMap[g_Context->CurrentFont];

	const auto& Atlas   = g_Context->Fonts[FontIndex];
	const auto& Metrics = Atlas.Metrics;

	const f32 Scale = PixelSize / Metrics.LineHeight;

	const f32 AtlasWidth  = (f32)Atlas.Width;
	const f32 AtlasHeight = (f32)Atlas.Height;

	while (*Char != 0)
	{
		i32  Codepoint = (u32)(*Char);
		auto Iterator  = Atlas.Glyphs.find(Codepoint);

		if (*Char == U'\n' || *Char == U'\r')
		{
			bool CheckCRLF = *Char == U'\r';

			CursorX = X;
			CursorY -= Metrics.LineHeight * Scale;

			if (CheckCRLF && *(Char + 1) == U'\n')
			{
				++Char;
			}
		}
		else if (Iterator != Atlas.Glyphs.end())
		{
			auto Glyph = Iterator->second;
			if (Glyph.HasGeometry)
			{
				const f32 Left = Glyph.PlaneBounds.Left, Right = Glyph.PlaneBounds.Right;
				const f32 Bottom = Glyph.PlaneBounds.Bottom, Top = Glyph.PlaneBounds.Top;

				const f32 GlyphWidth  = Right - Left;
				const f32 GlyphHeight = Top - Bottom;

				const f32 TexLeft = Glyph.AtlasBounds.Left, TexRight = Glyph.AtlasBounds.Right;
				const f32 TexBottom = AtlasHeight - Glyph.AtlasBounds.Bottom, TexTop = AtlasHeight - Glyph.AtlasBounds.Top;

				glyph_data WrittenGlyph;
				WrittenGlyph.Position    = vec2(CursorX, CursorY);
				WrittenGlyph.Scale       = vec2(GlyphWidth * 0.5f, GlyphHeight * 0.5f);
				WrittenGlyph.Translate   = vec2(Left, Bottom + WrittenGlyph.Scale.y);
				WrittenGlyph.Texcoords   = vec4(TexLeft / AtlasWidth, TexBottom / AtlasHeight, TexRight / AtlasWidth, TexTop / AtlasHeight);
				WrittenGlyph.GlobalScale = Scale;
				WrittenGlyph.TextureIndex = FontIndex;

				g_Context->GlyphData.push_back(WrittenGlyph);
			}

			CursorX += (Glyph.Advance * Scale);
		}

		++Char;
	}
}

}
