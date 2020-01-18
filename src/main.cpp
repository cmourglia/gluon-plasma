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

#include "logo.h"

static bool s_ShowStats   = false;
static bool s_EnableVSync = false;
static int  s_MsaaLevel   = 0;

static uint32_t s_WindowWidth  = 1024;
static uint32_t s_WindowHeight = 768;

const bgfx::ViewId kRenderView = 0;

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

	bgfx::setViewClear(kRenderView, BGFX_CLEAR_COLOR);
	bgfx::setViewRect(kRenderView, 0, 0, bgfx::BackbufferRatio::Equal);

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();

		bgfx::touch(kRenderView);

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats, F2 to toggle vsync, F3 to walk through MSAA settings");

		// Enable stats or debug text.
		bgfx::setDebug(s_ShowStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
		// Advance to next frame. Process submitted rendering primitives.
		bgfx::frame();
	}

	return 0;
}
