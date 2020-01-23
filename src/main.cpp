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

static bool s_ShowStats   = false;
static bool s_EnableVSync = false;
static int  s_MsaaLevel   = 0;

static uint32_t s_WindowWidth  = 1024;
static uint32_t s_WindowHeight = 768;

const bgfx::ViewId kRenderView = 0;

struct pos_color_vertex
{
	glm::vec2 Position;
	glm::vec4 Color;

	static void Init()
	{
		ms_Layout.begin()
		    .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
		    .end();
	}

	static bgfx::VertexLayout ms_Layout;
};

bgfx::VertexLayout pos_color_vertex::ms_Layout = bgfx::VertexLayout();

constexpr eastl::array<pos_color_vertex, 4> s_QuadVertices = {
    pos_color_vertex{glm::vec2{-50.0f, -50.0f}, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
    pos_color_vertex{glm::vec2{-50.0f, +50.0f}, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
    pos_color_vertex{glm::vec2{+50.0f, +50.0f}, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}},
    pos_color_vertex{glm::vec2{+50.0f, -50.0f}, glm::vec4{1.0f, 0.0f, 1.0f, 1.0f}},
};

constexpr eastl::array<uint16_t, 6> s_QuadIndices = {0, 2, 1, 0, 3, 2};

uint32_t GetResetFlags()
{
	uint32_t Flags = BGFX_RESET_NONE;
	if (s_EnableVSync)
		Flags |= BGFX_RESET_VSYNC;

	switch (s_MsaaLevel)
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
	bgfx::reset(s_WindowWidth, s_WindowHeight, GetResetFlags());
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
		s_ShowStats = !s_ShowStats;
	}

	if (Key == GLFW_KEY_F2 && Action == GLFW_RELEASE)
	{
		s_EnableVSync = !s_EnableVSync;
		ResizeWindow();
	}

	if (Key == GLFW_KEY_F3 && Action == GLFW_RELEASE)
	{
		s_MsaaLevel = ++s_MsaaLevel % 5;
		ResizeWindow();
	}

	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(Window, true);
	}
}

static void ResizeCallback(GLFWwindow* Window, int Width, int Height)
{
	s_WindowWidth  = Width;
	s_WindowHeight = Height;

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
	GLFWwindow* Window = glfwCreateWindow(s_WindowWidth, s_WindowHeight, "Hello BGFX", nullptr, nullptr);

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
	Init.resolution.width = s_WindowWidth = Width;
	Init.resolution.height = s_WindowHeight = Height;
	Init.resolution.reset                   = BGFX_RESET_NONE; // BGFX_RESET_VSYNC to enable vsync

	if (!bgfx::init(Init))
	{
		return 1;
	}

	bgfx::setViewClear(kRenderView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
	bgfx::setViewRect(kRenderView, 0, 0, bgfx::BackbufferRatio::Equal);

	pos_color_vertex::Init();
	uint32_t VertexBufferSize = (uint32_t)s_QuadVertices.size() * sizeof(pos_color_vertex);
	uint32_t IndexBufferSize  = (uint32_t)s_QuadIndices.size() * sizeof(uint16_t);
	auto VertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(s_QuadVertices.data(), VertexBufferSize), pos_color_vertex::ms_Layout);
	auto IndexBufferHandle  = bgfx::createIndexBuffer(bgfx::makeRef(s_QuadIndices.data(), IndexBufferSize));

	auto VertexShaderHandle   = LoadShader("shaders/bin/test.vert.bin");
	auto FragmentShaderHandle = LoadShader("shaders/bin/test.frag.bin");
	auto ProgramHandle        = bgfx::createProgram(VertexShaderHandle, FragmentShaderHandle, true);

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();

		bgfx::touch(kRenderView);

		const float HalfWidth  = s_WindowWidth * 0.5f;
		const float HalfHeight = s_WindowHeight * 0.5f;
		const auto  ProjMatrix = glm::orthoLH_ZO(-HalfWidth, HalfWidth, -HalfHeight, HalfHeight, 0.0f, 100.0f);
		bgfx::setViewTransform(0, nullptr, glm::value_ptr(ProjMatrix));

		bgfx::setVertexBuffer(0, VertexBufferHandle);
		bgfx::setIndexBuffer(IndexBufferHandle);
		// bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA);
		bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::submit(0, ProgramHandle);

		// Debug
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats, F2 to toggle vsync, F3 to walk through MSAA settings");
		// Enable stats or debug text.
		bgfx::setDebug(s_ShowStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

		// Advance to next frame. Process submitted rendering primitives.
		bgfx::frame();
	}

	bgfx::destroy(ProgramHandle);
	bgfx::destroy(VertexBufferHandle);
	bgfx::destroy(IndexBufferHandle);
	bgfx::shutdown();

	return 0;
}
