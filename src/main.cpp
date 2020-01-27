#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <glfw/glfw3.h>
#if BX_PLATFORM_LINUX
#	define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <glfw/glfw3native.h>

#include <loguru.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <eastl/array.h>
#include <eastl/vector.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <optick.h>

static bool gShowStats   = false;
static bool gEnableVSync = false;
static int  gMsaaLevel   = 0;

static uint32_t gWindowWidth  = 1024;
static uint32_t gWindowHeight = 768;

const bgfx::ViewId kRenderView = 0;

struct pos_texcoord_vertex
{
	glm::vec2 Position;
	// glm::vec2 Coords;
	// glm::vec4 Color;

	static void Init()
	{
		sLayout.begin()
		    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		    // .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    // .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
		    .end();
	}

	static bgfx::VertexLayout sLayout;
};

bgfx::VertexLayout pos_texcoord_vertex::sLayout = bgfx::VertexLayout();

constexpr eastl::array<pos_texcoord_vertex, 4> kQuadVertices = {
    pos_texcoord_vertex{glm::vec2{-1.0f, -1.0f} /*, glm::vec2{1.0f, 0.0f}*/},
    pos_texcoord_vertex{glm::vec2{-1.0f, 1.0f} /*, glm::vec2{0.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, 1.0f} /*, glm::vec2{1.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, -1.0f} /*, glm::vec2{1.0f, 0.0f}*/},
};

constexpr eastl::array<uint16_t, 6> kQuadIndices = {0, 2, 1, 0, 3, 2};

static bgfx::VertexBufferHandle gVertexBufferHandle = BGFX_INVALID_HANDLE;
static bgfx::IndexBufferHandle  gIndexBufferHandle  = BGFX_INVALID_HANDLE;

static bgfx::InstanceDataBuffer gInstanceDataBuffer;

static bgfx::UniformHandle             ParamsHandle          = BGFX_INVALID_HANDLE;
static bgfx::DynamicVertexBufferHandle PositionBuffer        = BGFX_INVALID_HANDLE;
static bgfx::DynamicVertexBufferHandle FillColorRadiusBuffer = BGFX_INVALID_HANDLE;
static bgfx::DynamicVertexBufferHandle BorderColorSizeBuffer = BGFX_INVALID_HANDLE;
static bgfx::IndirectBufferHandle      IndirectBufferHandle  = BGFX_INVALID_HANDLE;

using color = glm::vec4;

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

	bgfx::ProgramHandle Program;
};

void DrawRectangle(rendering_context* Context,
                   float              X,
                   float              Y,
                   float              Width,
                   float              Height,
                   color              Color,
                   color              BorderColor = color(0.0f, 0.0f, 0.0f, 1.0f))
{
	OPTICK_EVENT();

	rectangle Rectangle;
	Rectangle.Position    = glm::vec2(X, Y);
	Rectangle.Size        = glm::vec2(Width, Height);
	Rectangle.BorderColor = BorderColor;
	Rectangle.FillColor   = Color;

	Context->Rectangles.push_back(Rectangle);
	Context->Positions.push_back(glm::vec4(X, Y, Width, Height));
}

template <typename data_t>
inline void UpdateBuffer(bgfx::DynamicVertexBufferHandle Handle, const eastl::vector<data_t>& Data)
{
	bgfx::update(Handle, 0, bgfx::copy(Data.data(), (uint32_t)Data.size() * sizeof(data_t)));
}

void Flush(rendering_context* Context)
{
	if (!bgfx::isValid(gVertexBufferHandle))
	{
		pos_texcoord_vertex::Init();
		uint32_t VertexBufferSize = (uint32_t)kQuadVertices.size() * sizeof(pos_texcoord_vertex);
		uint32_t IndexBufferSize  = (uint32_t)kQuadIndices.size() * sizeof(uint16_t);

		gVertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(kQuadVertices.data(), VertexBufferSize), pos_texcoord_vertex::sLayout);
		gIndexBufferHandle  = bgfx::createIndexBuffer(bgfx::makeRef(kQuadIndices.data(), IndexBufferSize));
	}

	bgfx::getAvailInstanceDataBuffer(Context->Rectangles.size(), 16);
	bgfx::allocInstanceDataBuffer(&gInstanceDataBuffer, Context->Rectangles.size(), 16);

	memcpy(gInstanceDataBuffer.data, Context->Positions.data(), gInstanceDataBuffer.size);

	bgfx::setVertexBuffer(0, gVertexBufferHandle);
	bgfx::setIndexBuffer(gIndexBufferHandle);
	bgfx::setInstanceDataBuffer(&gInstanceDataBuffer);

	// bgfx::setBuffer(0, PositionBuffer, bgfx::Access::Read);
	// UpdateBuffer(PositionBuffer, Context->Positions);
	// bgfx::setInstanceDataBuffer(PositionBuffer, 0, Context->Rectangles.size());

	// for (auto Rectangle : Context->Rectangles)
	// {
	// 	bgfx::setVertexBuffer(0, gVertexBufferHandle);
	// 	bgfx::setIndexBuffer(gIndexBufferHandle);

	// 	glm::mat4 Transform = glm::mat4(1.0f);
	// 	Transform           = glm::translate(Transform, glm::vec3(Rectangle.Position, 0.0f));
	// 	Transform           = glm::scale(Transform, glm::vec3(Rectangle.Size, 1.0f));
	// 	bgfx::setTransform(glm ::value_ptr(Transform));

	// 	glm::vec4 Params[2];
	// 	Params[0] = Rectangle.FillColor;
	// 	Params[1] = glm::vec4(Rectangle.Position, Rectangle.Size);
	// 	bgfx::setUniform(ParamsHandle, &Params[0], 2);

	bgfx::setState(BGFX_STATE_WRITE_RGB);
	// bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_ALWAYS |
	//    BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
	bgfx::submit(0, Context->Program);
	// }

	// glm::vec4 Params[2];
	// Params[0] = Rectangle.FillColor;
	// Params[1] = glm::vec4(Rectangle.Position, Rectangle.Size);
	// bgfx::setUniform(ParamsHandle, &Params[0], 2);

	// bgfx::submit(0, Context->Program, IndirectBufferHandle, 0, Context->Rectangles.size());

	// bgfx::bgfx::setUniform(gColorUniform, glm::value_ptr(Color));
	// bgfx::setUniform(gBoxUniform, glm::value_ptr(glm::vec4(X, Y, X + Width, Y + Height)));
	// bgfx::setUniform(gSigmaUniform, glm::value_ptr(glm::vec4(Sigma)));

	// bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_ALWAYS |
	//    BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));

	// bgfx::setBuffer(0, PositionBuffer, bgfx::Access::Read);
	// bgfx::setBuffer(1, FillColorRadiusBuffer, bgfx::Access::Read);
	// bgfx::setBuffer(2, BorderColorSizeBuffer, bgfx::Access::Read);

	// UpdateBuffer(PositionBuffer, Context->Positions);
	// UpdateBuffer(FillColorRadiusBuffer, Context->FillColorRadii);
	// UpdateBuffer(BorderColorSizeBuffer, Context->BorderColorSize);

	Context->Rectangles.clear();
	Context->Positions.clear();
}

uint32_t GetResetFlags()
{
	uint32_t Flags = BGFX_RESET_NONE;
	// if (gEnableVSync)
	// 	Flags |= BGFX_RESET_VSYNC;

	// switch (gMsaaLevel)
	// {
	// 	case 1:
	// 		Flags |= BGFX_RESET_MSAA_X2;
	// 		break;
	// 	case 2:
	// 		Flags |= BGFX_RESET_MSAA_X4;
	// 		break;
	// 	case 3:
	// 		Flags |= BGFX_RESET_MSAA_X8;
	// 		break;
	// 	case 4:
	// 		Flags |= BGFX_RESET_MSAA_X16;
	// 		break;
	// }

	return Flags;
}

static void ResizeWindow()
{
	bgfx::reset(gWindowWidth, gWindowHeight, GetResetFlags());
}

static void ErrorCallback(int Error, const char* Description)
{
	LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description);
}

int   gElemCount = 1;
float gDelta     = 2.0f / gElemCount;
float gRadius    = gDelta / 2.0f;

eastl::vector<color> gColors;

void SetColors()
{
	auto GetRandomColor = []() {
		color Color = color((float)rand(), (float)rand(), (float)rand(), 1.0);
		Color /= (float)RAND_MAX;
		Color.w = 1.0f;
		return Color;
	};

	gColors.resize(gElemCount * gElemCount);
	for (auto& Color : gColors)
	{
		Color = GetRandomColor();
	}
}

static void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
	if (Key == GLFW_KEY_F1 && Action == GLFW_RELEASE)
	{
		gShowStats = !gShowStats;
	}

	if (Key == GLFW_KEY_F2 && Action == GLFW_RELEASE)
	{
		gEnableVSync = !gEnableVSync;
		ResizeWindow();
	}

	if (Key == GLFW_KEY_F3 && Action == GLFW_RELEASE)
	{
		gMsaaLevel = ++gMsaaLevel % 5;
		ResizeWindow();
	}

	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(Window, true);
	}
}

static void CharCallback(GLFWwindow* Window, unsigned int Codepoint)
{
	if (Codepoint == '+')
	{
		gElemCount *= 2;
		gElemCount = eastl::min(gElemCount, 512);
	}

	if (Codepoint == '-')
	{
		gElemCount /= 2;
		gElemCount = eastl::max(1, gElemCount);
	}

	SetColors();

	gDelta  = 2.0f / gElemCount;
	gRadius = gDelta / 2.0f;
}

static void ResizeCallback(GLFWwindow* Window, int Width, int Height)
{
	gWindowWidth  = Width;
	gWindowHeight = Height;

	ResizeWindow();
}

static bgfx::ShaderHandle LoadShader(const char* ShaderName)
{
	FILE* File = fopen(ShaderName, "rb");
	if (!File)
	{
		return BGFX_INVALID_HANDLE;
	}

	fseek(File, 0, SEEK_END);
	size_t Size = ftell(File);
	fseek(File, 0, SEEK_SET);

	char* Data = (char*)malloc(Size + 1);
	if (!Data)
	{
		fclose(File);
		return BGFX_INVALID_HANDLE;
	}

	Size       = fread(Data, sizeof(char), Size, File);
	Data[Size] = '\0';

	const bgfx::Memory* Memory = bgfx::copy(Data, (uint32_t)Size + 1);
	bgfx::ShaderHandle  Handle = bgfx::createShader(Memory);
	bgfx::setName(Handle, ShaderName);

	free(Data);
	fclose(File);

	return Handle;
}

int main()
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
	{
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* Window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Hello BGFX", nullptr, nullptr);

	if (!Window)
	{
		return 1;
	}

	glfwSetKeyCallback(Window, KeyCallback);
	glfwSetCharCallback(Window, CharCallback);
	glfwSetWindowSizeCallback(Window, ResizeCallback);

	// Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
	// Most graphics APIs must be used on the same thread that created the window.
	bgfx::renderFrame();

	// Initialize bgfx using the native window handle and window resolution.
	bgfx::Init Init = {};
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	Init.platformData.ndt = glfwGetX11Display();
	Init.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(Window);
#elif BX_PLATFORM_OSX
	Init.platformData.nwh = glfwGetCocoaWindow(Window);
#elif BX_PLATFORM_WINDOWS
	Init.platformData.nwh = glfwGetWin32Window(Window);
#endif

	int Width, Height;
	glfwGetWindowSize(Window, &Width, &Height);
	gWindowWidth  = Width;
	gWindowHeight = Height;

	Init.resolution.width  = gWindowWidth;
	Init.resolution.height = gWindowHeight;
	Init.resolution.reset  = BGFX_RESET_NONE; // BGFX_RESET_VSYNC to enable vsync

#if BX_PLATFORM_WINDOWS
	Init.type = bgfx::RendererType::Direct3D11;
#else
	Init.type             = bgfx::RendererType::OpenGL;
#endif

	if (!bgfx::init(Init))
	{
		return 1;
	}

	bgfx::setViewClear(kRenderView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff);
	bgfx::setViewRect(kRenderView, 0, 0, bgfx::BackbufferRatio::Equal);

	auto VertexShaderHandle   = LoadShader("shaders/bin/test.vert.bin");
	auto FragmentShaderHandle = LoadShader("shaders/bin/test.frag.bin");
	auto ProgramHandle        = bgfx::createProgram(VertexShaderHandle, FragmentShaderHandle, true);

	ParamsHandle = bgfx::createUniform("Params", bgfx::UniformType::Vec4, 2);

	bgfx::VertexLayout VertexLayout;
	VertexLayout.begin().add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float).end();

	PositionBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// FillColorRadiusBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// BorderColorSizeBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	IndirectBufferHandle = bgfx::createIndirectBuffer(256);

	rendering_context Context;
	Context.Program = ProgramHandle;

	SetColors();

	while (!glfwWindowShouldClose(Window))
	{
		OPTICK_FRAME("MainThread");

		glfwPollEvents();

		bgfx::setViewRect(0, 0, 0, (uint16_t)gWindowWidth, (uint16_t)gWindowHeight);
		bgfx::touch(kRenderView);

		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)gWindowWidth, (float)gWindowHeight, 0.0f, 0.0f, 100.0f);
		// const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)gWindowWidth, 0.0f, (float)gWindowHeight, 0.0f, 100.0f);
		bgfx::setViewTransform(0, glm::value_ptr(ViewMatrix), glm::value_ptr(ProjMatrix));

		// for (int i = 0; i < 10; ++i)
		// {
		// 	for (int j = 0; j < 10; ++j)
		// 	{
		// 		float x = -1.0f +
		// 		DrawRectangle(&Context,
		// 		              0.0f,
		// 		              0.0f,
		// 		              0.5f,
		// 		              0.5f,
		// 		              glm::vec4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.0f));
		// 	}
		// }

		for (int i = 0; i < gElemCount; ++i)
		{
			for (int j = 0; j < gElemCount; ++j)
			{
				const float x = -1.0f + gDelta * i + gRadius;
				const float y = -1.0f + gDelta * j + gRadius;

				const float FinalX      = (x + 1.f) * 0.5f * gWindowWidth;
				const float FinalY      = (y + 1.f) * 0.5f * gWindowHeight;
				const float FinalRadius = gRadius * eastl::min(gWindowWidth, gWindowHeight) * 0.5;

				DrawRectangle(&Context, FinalX, FinalY, FinalRadius, FinalRadius, gColors[i * gElemCount + j]);
			}
		}
		// DrawRectangle(&Context, gWindowWidth / 2, gWindowHeight / 2, 100, 100, color(0.25, 0.55, 0.66, 1.0));
		// DrawRectangle(&Context, -0.5f, -0.5f, 0.5f, 0.5f, GetRandomColor());
		// DrawRectangle(&Context, -0.5f, 0.5f, 0.5f, 0.5f, GetRandomColor());
		// DrawRectangle(&Context, 0.5f, -0.5f, 0.5f, 0.5f, GetRandomColor());
		// DrawRectangle(&Context, 0.5f, 0.5f, 0.5f, 0.5f, GetRandomColor());

		Flush(&Context);
		// DrawRectangle(ProgramHandle, 0, 0, gWindowWidth, gWindowHeight, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
		// DrawRectangle(ProgramHandle, 50, 50, 50, 50, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		// DrawRectangle(ProgramHandle, 200, 50, 50, 50, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		// DrawRectangle(ProgramHandle, 50, 200, 50, 50, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		// DrawRectangle(ProgramHandle, 200, 200, 50, 50, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

		// Debug
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats, F2 to toggle vsync, F3 to walk through MSAA settings");
		// Enable stats or debug text.
		bgfx::setDebug(gShowStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

		// Advance to next frame. Process submitted rendering primitives.
		bgfx::frame();
	}

	// bgfx::destroy(Params);

	// bgfx::destroy(ProgramHandle);

	// bgfx::destroy(gVertexBufferHandle);
	// bgfx::destroy(gIndexBufferHandle);

	bgfx::shutdown();

	OPTICK_SHUTDOWN();

	return 0;
}
