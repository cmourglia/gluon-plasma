#include "gln_application.h"

#include <loguru.hpp>

#include <gluon/core/gln_defines.h>

#include <gluon/api/gln_application_p.h>
#include <gluon/api/gln_widgets.h>
#include <gluon/api/gln_renderer_p.h>

#include <EASTL/vector.h>

#include <GLFW/glfw3.h>

namespace gluon
{
application* application::s_Application = nullptr;

static void ErrorCallback(i32 Error, const char* Description) { LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description); }

bool application_impl::ShouldClose() const
{
	bool Result = true;
	for (const auto& Window : Windows)
	{
		Result = Result && WindowShouldClose(Window);
	}

	return Result;
}

void RegisterWindow(window* Window) { application::Get()->m_Impl->Windows.push_back(Window); }

application* application::Get() { return s_Application; }

application::application()
{
	GLN_ASSERT(s_Application == nullptr);

	if (s_Application != nullptr)
	{
		LOG_F(ERROR, "There can be only one application");
		return;
	}

	s_Application = this;

	m_Impl = new application_impl();

	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
	{
		LOG_F(FATAL, "Cannot initialize GLFW");
	}
}

application::~application()
{
	glfwTerminate();
	delete m_Impl;
}

// void* application::GetNativeHandle() const
// {
// #if GLN_PLATFORM_WINDOWS
// 	return glfwGetWin32Window(m_Impl->Window);
// #elif GLN_PLATFORM_LINUX
// 	return (void*)(uintptr_t)glfGetX11Window(m_Impl->Window);
// #endif
// }

// vec2i application::GetSize() const
// {
// 	i32 Width, Height;
// 	glfwGetWindowSize(m_Impl->Window, &Width, &Height);
// 	return vec2i(Width, Height);
// }

i32 application::Run()
{
	while (!m_Impl->ShouldClose())
	{
		glfwPollEvents();

		for (auto&& Window : m_Impl->Windows)
		{
			Window->Traverse();
		}

		gluon::priv::Flush();

		for (auto&& Window : m_Impl->Windows)
		{
			SwapBuffers(Window);
		}
	}

	return 0;
}
}
