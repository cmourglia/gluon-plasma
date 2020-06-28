#include "renderer.h"
#include "render_backend.h"
#include "gln_math.h"
#include "gln_text.h"

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

#include <loguru.hpp>

#include <optick.h>

namespace gln
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

struct draw_command
{
	u32 VertexCount;
	u32 InstanceCount;
	u32 FirstIndex;
	u32 BaseVertex;
	u32 BaseInstance;
};

namespace fs = std::filesystem;

static fs::file_time_type g_RectVShaderTime;
static fs::file_time_type g_RectFShaderTime;
static fs::file_time_type g_TextVShaderTime;
static fs::file_time_type g_TextFShaderTime;

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

	{
		g_RectVShaderTime = fs::last_write_time(fs::path("shaders/rect.vert.glsl"));
		g_RectFShaderTime = fs::last_write_time(fs::path("shaders/rect.frag.glsl"));

		auto VertexShaderHandle   = LoadShader("shaders/rect.vert.glsl", ShaderType_Vertex);
		auto FragmentShaderHandle = LoadShader("shaders/rect.frag.glsl", ShaderType_Fragment);
		auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

		Context->RectProgram = ProgramHandle;

		pos_vertex::Init();
		u32 VertexBufferSize = (u32)k_QuadVertices.size() * sizeof(pos_vertex);
		u32 IndexBufferSize  = (u32)k_QuadIndices.size() * sizeof(uint16_t);

		Context->RectVertexBuffer = CreateImmutableBuffer(k_QuadVertices.size() * sizeof(pos_vertex), k_QuadVertices.data());
		Context->RectIndexBuffer  = CreateImmutableBuffer(k_QuadIndices.size() * sizeof(uint16_t), k_QuadIndices.data());

		Context->RectVertexArray = CreateVertexArray(Context->RectIndexBuffer);
		AttachVertexBuffer(Context->RectVertexArray, Context->RectVertexBuffer, pos_vertex::s_Layout);

		Context->LastRectangleCount   = 128 * 128;
		Context->RectangleInfoSSBO    = CreateImmutableBuffer(128 * 128 * sizeof(rectangle));
		Context->RectangleInfoSSBOPtr = MapBuffer(Context->RectangleInfoSSBO);
	}

	{
		g_TextVShaderTime = fs::last_write_time(fs::path("shaders/text.vert.glsl"));
		g_TextFShaderTime = fs::last_write_time(fs::path("shaders/text.frag.glsl"));

		auto VertexShaderHandle   = LoadShader("shaders/text.vert.glsl", ShaderType_Vertex);
		auto FragmentShaderHandle = LoadShader("shaders/text.frag.glsl", ShaderType_Fragment);
		auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

		Context->TextProgram = ProgramHandle;
	}

	return Context;
}

void DestroyRenderingContext(rendering_context* Context)
{
	if (glIsProgram(Context->RectProgram.Idx))
	{
		glDeleteProgram(Context->RectProgram.Idx);
	}
	delete Context;
}

void Resize(rendering_context* Context, f32 Width, f32 Height)
{
	Context->ViewportWidth  = Width;
	Context->ViewportHeight = Height;
}

void SetTextScale(rendering_context* Context, f32 ScaleX, f32 ScaleY)
{
	Context->TextScaleX = ScaleX;
	Context->TextScaleY = ScaleY;
}

void SetCameraInfo(rendering_context* Context, const f32* ViewMatrix, const f32* ProjMatrix)
{
	memcpy(Context->ViewMatrix, ViewMatrix, 16 * sizeof(f32));
	memcpy(Context->ProjMatrix, ProjMatrix, 16 * sizeof(f32));
}

void DrawRectangle(rendering_context* Context,
                   f32                X,
                   f32                Y,
                   f32                Width,
                   f32                Height,
                   color              Color,
                   f32                Radius /* = 0.0f */,
                   f32                BorderWidth /*= 1.0f*/,
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
}

void SetFont(rendering_context* Context, const char* FontName)
{
	Context->CurrentFont = FontName;

	// Load FontData if needed
	if (Context->FontLookupMap.find(FontName) == Context->FontLookupMap.end())
	{
		auto Atlas = LoadFontAtlas(FontName);

		// FIXME: We probably do not want to upload textures at this point
		texture_handle Texture = CreateTexture(Atlas.Width, Atlas.Height, 3);
		SetTextureWrapping(Texture, WrapMode_ClampToBorder, WrapMode_ClampToBorder);
		SetTextureData(Texture, Atlas.Data);

		Context->FontLookupMap[FontName] = (u32)Context->Fonts.size();
		Context->FontTextures.push_back(Texture);
		Context->Fonts.push_back(std::move(Atlas));
	}
}

void DrawText(rendering_context* Context, const char32_t* Text, f32 PixelSize, f32 X, f32 Y, color FillColor)
{
	const char32_t* Char = Text;

	f32 CursorX = X;
	f32 CursorY = Y;

	const u32 FontIndex = Context->FontLookupMap[Context->CurrentFont];

	const auto& Atlas   = Context->Fonts[FontIndex];
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

				Context->GlyphData.push_back(WrittenGlyph);
			}

			CursorX += (Glyph.Advance * Scale);
		}

		++Char;
	}
}

void RenderRectangles(rendering_context* Context)
{
	OPTICK_EVENT();

	const i32 RectangleCount = (i32)Context->Rectangles.size();

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

		auto VertexShaderHandle   = LoadShader("shaders/rect.vert.glsl", ShaderType_Vertex);
		auto FragmentShaderHandle = LoadShader("shaders/rect.frag.glsl", ShaderType_Fragment);
		auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

		Context->RectProgram = ProgramHandle;
	}

	glUseProgram(Context->RectProgram.Idx);

	const i32 ViewLoc         = glGetUniformLocation(Context->RectProgram.Idx, "u_View");
	const i32 ProjLoc         = glGetUniformLocation(Context->RectProgram.Idx, "u_Proj");
	const i32 ViewportSizeLoc = glGetUniformLocation(Context->RectProgram.Idx, "u_ViewportSize");

	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, Context->ViewMatrix);
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, Context->ProjMatrix);
	glUniform2f(ViewportSizeLoc, Context->ViewportWidth, Context->ViewportHeight);

	if (Context->LastRectangleCount < RectangleCount)
	{
		UnmapBuffer(Context->RectangleInfoSSBO);
		ResizeImmutableBuffer(&Context->RectangleInfoSSBO, RectangleCount * sizeof(rectangle));

		Context->LastRectangleCount   = RectangleCount;
		Context->RectangleInfoSSBOPtr = MapBuffer(Context->RectangleInfoSSBO);
	}

	memcpy(Context->RectangleInfoSSBOPtr, Context->Rectangles.data(), RectangleCount * sizeof(rectangle));

	glBindVertexArray(Context->RectVertexArray.Idx);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Context->RectangleInfoSSBO.Idx);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, RectangleCount);

	Context->Rectangles.clear();
}

void RenderTexts(rendering_context* Context)
{
	OPTICK_EVENT();

	const i32 GlyphCount = (i32)Context->GlyphData.size();

	auto VertexTime   = fs::last_write_time(fs::path("shaders/text.vert.glsl"));
	auto FragmentTime = fs::last_write_time(fs::path("shaders/text.frag.glsl"));

	if (VertexTime > g_TextVShaderTime || FragmentTime > g_TextFShaderTime)
	{
		g_TextVShaderTime = VertexTime;
		g_TextFShaderTime = FragmentTime;

		auto VertexShaderHandle   = LoadShader("shaders/text.vert.glsl", ShaderType_Vertex);
		auto FragmentShaderHandle = LoadShader("shaders/text.frag.glsl", ShaderType_Fragment);
		auto ProgramHandle        = CreateProgram(VertexShaderHandle, FragmentShaderHandle, true);

		Context->TextProgram = ProgramHandle;
	}

	glUseProgram(Context->TextProgram.Idx);

	const i32 ViewLoc         = glGetUniformLocation(Context->TextProgram.Idx, "u_View");
	const i32 ProjLoc         = glGetUniformLocation(Context->TextProgram.Idx, "u_Proj");
	const i32 ViewportSizeLoc = glGetUniformLocation(Context->TextProgram.Idx, "u_ViewportSize");

	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, Context->ViewMatrix);
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, Context->ProjMatrix);
	glUniform2f(ViewportSizeLoc, Context->ViewportWidth, Context->ViewportHeight);

	if (Context->LastGlyphCount < GlyphCount)
	{
		UnmapBuffer(Context->TextInfoSSBO);
		ResizeImmutableBuffer(&Context->TextInfoSSBO, GlyphCount * sizeof(glyph_data));

		Context->LastGlyphCount  = GlyphCount;
		Context->TextInfoSSBOPtr = MapBuffer(Context->TextInfoSSBO);
	}

	memcpy(Context->TextInfoSSBOPtr, Context->GlyphData.data(), GlyphCount * sizeof(glyph_data));

	glBindVertexArray(Context->RectVertexArray.Idx);

	u32 CurrentIndex = 0;
	for (const auto& Font : Context->FontTextures)
	{
		glBindTextureUnit(CurrentIndex++, Font.Idx);
	}

	i32 TextureBindings[] = {0, 1, 2, 3, 4, 5, 6, 7};

	const i32 TexturesLoc = glGetUniformLocation(Context->TextProgram.Idx, "u_Textures");
	glUniform1iv(TexturesLoc, 8, TextureBindings);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Context->TextInfoSSBO.Idx);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, GlyphCount);

	Context->GlyphData.clear();
}

void Flush(rendering_context* Context)
{
	OPTICK_EVENT();

	glClearColor(0.2f, 0.4f, 0.5f, 1.0f);
	glViewport(0, 0, (GLsizei)Context->ViewportWidth, (GLsizei)Context->ViewportHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	RenderRectangles(Context);
	RenderTexts(Context);

	glDisable(GL_BLEND);
}

}
