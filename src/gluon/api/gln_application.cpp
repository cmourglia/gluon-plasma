#include "gln_application.h"

#include <loguru.hpp>

#include <gluon/core/gln_defines.h>
#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_renderer_p.h>

#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif

#if GLN_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#elif GLN_PLATFORM_LINUX
#	define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_GLX
#endif
#include <GLFW/glfw3native.h>

namespace gluon
{
static void ErrorCallback(i32 Error, const char* Description) { LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description); }

static void ResizeCallback(GLFWwindow* Window, i32 Width, i32 Height)
{
	application* App = (application*)glfwGetWindowUserPointer(Window);

	gluon::priv::Resize(Width, Height);

	App->OnResize(Width, Height);
}

static void CursorPosCallback(GLFWwindow* Window, f64 X, f64 Y)
{
	application* App = (application*)glfwGetWindowUserPointer(Window);
	App->OnMouseMove((f32)X, (f32)Y);
}

static void MouseButtonCallback(GLFWwindow* Window, i32 Button, i32 Action, i32 Mods)
{
	application* App = (application*)glfwGetWindowUserPointer(Window);
	App->OnMouseEvent((input_mouse_buttons)Button, (input_actions)Action, (input_mods)Mods);
}

static void KeyCallback(GLFWwindow* Window, i32 Key, i32 ScanCode, i32 Action, i32 Mods)
{
	application* App = (application*)glfwGetWindowUserPointer(Window);
	App->OnKeyEvent((input_keys)Key, (input_actions)Action, (input_mods)Mods);
}

static void CharCallback(GLFWwindow* Window, u32 Codepoint)
{
	application* App = (application*)glfwGetWindowUserPointer(Window);
	App->OnCharInput(Codepoint);
}

struct application_impl
{
	GLFWwindow* Window;
};

application::application(const char* WindowTitle, i32 Width, i32 Height, bool Resizable)
{
	m_Impl = new application_impl();

	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
	{
		LOG_F(FATAL, "Cannot initialize GLFW");
	}

	glfwWindowHint(GLFW_RESIZABLE, Resizable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Impl->Window = glfwCreateWindow(Width, Height, WindowTitle, nullptr, nullptr);

	if (!m_Impl->Window)
	{
		LOG_F(FATAL, "Cannot create GLFW window");
	}

	glfwMakeContextCurrent(m_Impl->Window);
	glfwSwapInterval(0);

	gluon::priv::CreateRenderingContext();
}

application::~application()
{
	gluon::priv::DestroyRenderingContext();

	glfwSetWindowUserPointer(m_Impl->Window, nullptr);
	glfwDestroyWindow(m_Impl->Window);
	glfwTerminate();
	delete m_Impl;
}

void application::SetWindowTitle(const char* NewTitle) { glfwSetWindowTitle(m_Impl->Window, NewTitle); }

void application::SetSize(i32 Width, i32 Height)
{
	// TODO
}

void* application::GetNativeHandle() const
{
#if GLN_PLATFORM_WINDOWS
	return glfwGetWin32Window(m_Impl->Window);
#elif GLN_PLATFORM_LINUX
	return (void*)(uintptr_t)glfGetX11Window(m_Impl->Window);
#endif
}

vec2i application::GetSize() const
{
	i32 Width, Height;
	glfwGetWindowSize(m_Impl->Window, &Width, &Height);
	return vec2i(Width, Height);
}

void application::Exit() { glfwSetWindowShouldClose(m_Impl->Window, GLFW_TRUE); }

i32 application::Run()
{
	glfwSetWindowUserPointer(m_Impl->Window, (void*)this);

	glfwSetWindowSizeCallback(m_Impl->Window, ResizeCallback);

	glfwSetCursorPosCallback(m_Impl->Window, CursorPosCallback);
	glfwSetMouseButtonCallback(m_Impl->Window, MouseButtonCallback);
	glfwSetKeyCallback(m_Impl->Window, KeyCallback);
	glfwSetCharCallback(m_Impl->Window, CharCallback);

	glfwShowWindow(m_Impl->Window);

	while (!glfwWindowShouldClose(m_Impl->Window))
	{
		glfwPollEvents();

		OnUpdate();

		gluon::priv::Flush();

		glfwSwapBuffers(m_Impl->Window);
	}

	return 0;
}
}
