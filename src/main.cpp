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
    pos_texcoord_vertex{glm::vec2{0.0f, 0.0f} /*, glm::vec2{1.0f, 0.0f}*/},
    pos_texcoord_vertex{glm::vec2{0.0f, 1.0f} /*, glm::vec2{0.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, 1.0f} /*, glm::vec2{1.0f, 1.0f}*/},
    pos_texcoord_vertex{glm::vec2{1.0f, 0.0f} /*, glm::vec2{1.0f, 0.0f}*/},
};

constexpr eastl::array<uint16_t, 6> kQuadIndices = {0, 2, 1, 0, 3, 2};

static bgfx::VertexBufferHandle gVertexBufferHandle = BGFX_INVALID_HANDLE;
static bgfx::IndexBufferHandle  gIndexBufferHandle  = BGFX_INVALID_HANDLE;

static bgfx::UniformHandle gColorUniform = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle gBoxUniform   = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle gSigmaUniform = BGFX_INVALID_HANDLE;

using color = glm::vec4;

void DrawRectangle(bgfx::ProgramHandle Program, float X, float Y, float Width, float Height, color Color, bool bWithShadow = false)
{
	if (!bgfx::isValid(gVertexBufferHandle))
	{
		pos_texcoord_vertex::Init();
		uint32_t VertexBufferSize = (uint32_t)kQuadVertices.size() * sizeof(pos_texcoord_vertex);
		uint32_t IndexBufferSize  = (uint32_t)kQuadIndices.size() * sizeof(uint16_t);

		gVertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(kQuadVertices.data(), VertexBufferSize), pos_texcoord_vertex::sLayout);
		gIndexBufferHandle  = bgfx::createIndexBuffer(bgfx::makeRef(kQuadIndices.data(), IndexBufferSize));
	}

	glm::mat4 Transform = glm::mat4(1.0f);
	Transform           = glm::translate(Transform, glm::vec3(X, Y, 0.0f));
	Transform           = glm::scale(Transform, glm::vec3(Width, Height, 1.0f));
	bgfx::setTransform(glm ::value_ptr(Transform));

	bgfx::setVertexBuffer(0, gVertexBufferHandle);
	bgfx::setIndexBuffer(gIndexBufferHandle);

	float Sigma = 10.0f;

	bgfx::setUniform(gColorUniform, glm::value_ptr(Color));
	bgfx::setUniform(gBoxUniform, glm::value_ptr(glm::vec4(X, Y, X + Width, Y + Height)));
	bgfx::setUniform(gSigmaUniform, glm::value_ptr(glm::vec4(Sigma)));

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_ALWAYS |
	               BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
	bgfx::submit(0, Program);
}

uint32_t GetResetFlags()
{
	uint32_t Flags = BGFX_RESET_NONE;
	if (gEnableVSync)
		Flags |= BGFX_RESET_VSYNC;

	switch (gMsaaLevel)
	{
		case 1:
			Flags |= BGFX_RESET_MSAA_X2;
			break;
		case 2:
			Flags |= BGFX_RESET_MSAA_X4;
			break;
		case 3:
			Flags |= BGFX_RESET_MSAA_X8;
			break;
		case 4:
			Flags |= BGFX_RESET_MSAA_X16;
			break;
	}

	return Flags;
}

static void ResizeWindow()
{
	bgfx::reset(gWindowWidth, gWindowHeight, GetResetFlags());
	bgfx::setViewRect(kRenderView, 0, 0, bgfx::BackbufferRatio::Equal);
}

static void ErrorCallback(int Error, const char* Description)
{
	LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description);
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
	Init.resolution.width = gWindowWidth = Width;
	Init.resolution.height = gWindowHeight = Height;
	Init.resolution.reset                  = BGFX_RESET_NONE; // BGFX_RESET_VSYNC to enable vsync

	if (!bgfx::init(Init))
	{
		return 1;
	}

	bgfx::setViewClear(kRenderView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xdededeff);
	bgfx::setViewRect(kRenderView, 0, 0, bgfx::BackbufferRatio::Equal);

	auto VertexShaderHandle   = LoadShader("shaders/bin/test.vert.bin");
	auto FragmentShaderHandle = LoadShader("shaders/bin/test.frag.bin");
	auto ProgramHandle        = bgfx::createProgram(VertexShaderHandle, FragmentShaderHandle, true);

	gColorUniform = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
	gBoxUniform   = bgfx::createUniform("u_box", bgfx::UniformType::Vec4);
	gSigmaUniform = bgfx::createUniform("u_sigma", bgfx::UniformType::Vec4);

	while (!glfwWindowShouldClose(Window))
	{
		OPTICK_FRAME("MainThread");

		glfwPollEvents();

		bgfx::touch(kRenderView);

		// const auto ProjMatrix = glm::orthoLH_ZO(-(float)gWindowWidth / 2.0f,
		//                                         (float)gWindowWidth / 2.0f,
		//                                         -(float)gWindowHeight / 2.0f,
		//                                         (float)gWindowHeight / 2.0f,
		//                                         0.0f,
		//                                         100.0f);
		// const auto ViewMatrix = glm::rotate(glm::mat4(1.0f), 3.14f, glm::vec3(1.0f, 0.0f, 0.0f));
		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)gWindowWidth, (float)gWindowHeight, 0.0f, 0.0f, 100.0f);
		// const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)gWindowWidth, 0.0f, (float)gWindowHeight, 0.0f, 100.0f);
		bgfx::setViewTransform(0, glm::value_ptr(ViewMatrix), glm::value_ptr(ProjMatrix));

		// DrawRectangle(ProgramHandle, 0, 0, gWindowWidth, gWindowHeight, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
		// DrawRectangle(ProgramHandle, 50, 50, 50, 50, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		// DrawRectangle(ProgramHandle, 200, 50, 50, 50, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		// DrawRectangle(ProgramHandle, 50, 200, 50, 50, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		DrawRectangle(ProgramHandle, 200, 200, 50, 50, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

		// Debug
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats, F2 to toggle vsync, F3 to walk through MSAA settings");
		// Enable stats or debug text.
		bgfx::setDebug(gShowStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

		// Advance to next frame. Process submitted rendering primitives.
		bgfx::frame();
	}

	bgfx::destroy(gColorUniform);
	bgfx::destroy(ProgramHandle);
	bgfx::destroy(gVertexBufferHandle);
	bgfx::destroy(gIndexBufferHandle);
	bgfx::shutdown();

	OPTICK_SHUTDOWN();

	return 0;
}
