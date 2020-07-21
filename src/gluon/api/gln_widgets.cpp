#include <gluon/api/gln_widgets.h>

#include <gluon/core/gln_macros.h>

#include <gluon/api/gln_widgets_p.h>
#include <gluon/api/gln_application.h>
#include <gluon/api/gln_application_p.h>
#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_renderer_p.h>

#include <EASTL/vector.h>

#include <GLFW/glfw3.h>

#include <loguru.hpp>

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

static void ResizeCallback(GLFWwindow* Window, i32 Width, i32 Height)
{
	// application* App = (application*)glfwGetWindowUserPointer(Window);

	gluon::priv::Resize((f32)Width, (f32)Height);

	// App->OnResize(Width, Height);
}

static void CursorPosCallback(GLFWwindow* Window, f64 X, f64 Y)
{
	// application* App = (application*)glfwGetWindowUserPointer(Window);
	// App->OnMouseMove((f32)X, (f32)Y);
}

static void MouseButtonCallback(GLFWwindow* Window, i32 Button, i32 Action, i32 Mods)
{
	// application* App = (application*)glfwGetWindowUserPointer(Window);
	// App->OnMouseEvent((input_mouse_buttons)Button, (input_actions)Action, (input_mods)Mods);
}

static void KeyCallback(GLFWwindow* Window, i32 Key, i32 ScanCode, i32 Action, i32 Mods)
{
	// application* App = (application*)glfwGetWindowUserPointer(Window);
	// App->OnKeyEvent((input_keys)Key, (input_actions)Action, (input_mods)Mods);
}

static void CharCallback(GLFWwindow* Window, u32 Codepoint)
{
	// application* App = (application*)glfwGetWindowUserPointer(Window);
	// App->OnCharInput(Codepoint);
}

void widget_impl::ClearChildren()
{
	IsDeletingChildren = true;

	for (auto&& Child : Children)
	{
		ChildBeingDeleted = Child;
		delete Child;
		ChildBeingDeleted;
	}
	Children.clear();
	IsDeletingChildren = false;
}

bool WindowShouldClose(window* Window) { return glfwWindowShouldClose(Window->m_Window->Window); }
void SwapBuffers(window* Window) { glfwSwapBuffers(Window->m_Window->Window); }

widget::widget(widget* Parent)
{
	m_Widget = new widget_impl();

	if (Parent != nullptr)
	{
		SetParent(Parent);
	}
}

widget::~widget()
{
	// FIXME: Parent null then clear or vice versa ?
	SetParent(nullptr);
	m_Widget->ClearChildren();
}

void widget::SetParent(widget* Parent)
{
	GLN_ASSERT(Parent != this);

	if (m_Widget->Parent != nullptr)
	{
		widget_impl* OldParent = m_Widget->Parent->m_Widget;
		if (OldParent->ChildBeingDeleted != this)
		{
			const auto It = eastl::find(OldParent->Children.begin(), OldParent->Children.end(), this);
			if (OldParent->IsDeletingChildren)
			{
				// Parent's children are being deleted, set this to null to avoid deleting it.
				*It = nullptr;
			}
			else
			{
				// Simply remove this from the parent's hierarchy
				OldParent->Children.erase(It);
			}
		}
	}

	m_Widget->Parent = Parent;

	if (m_Widget->Parent != nullptr)
	{
		m_Widget->Parent->m_Widget->Children.push_back(this);
	}
}

void widget::Traverse()
{
	for (auto&& Child : m_Widget->Children)
	{
		Child->Traverse();
	}
}

window::window(const char* Title)
    : widget(nullptr)
{
	m_Window = new window_impl();

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if GLN_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	m_Window->Window = glfwCreateWindow(800, 600, Title, nullptr, nullptr);

	if (m_Window->Window == nullptr)
	{
		LOG_F(FATAL, "Cannot create GLFW window");
	}

	glfwSetWindowUserPointer(m_Window->Window, this);

	glfwSetWindowSizeCallback(m_Window->Window, ResizeCallback);
	glfwSetCursorPosCallback(m_Window->Window, CursorPosCallback);
	glfwSetMouseButtonCallback(m_Window->Window, MouseButtonCallback);
	glfwSetKeyCallback(m_Window->Window, KeyCallback);
	glfwSetCharCallback(m_Window->Window, CharCallback);

	glfwMakeContextCurrent(m_Window->Window);
	glfwSwapInterval(0);

	// glfwShowWindow(m_Impl->Window);

	// FIXME: The rendering context needs to somehow be attached to the window
	gluon::priv::CreateRenderingContext();

	RegisterWindow(this);

	i32 Width, Height;
	glfwGetFramebufferSize(m_Window->Window, &Width, &Height);
	m_Widget->Size = vec2((f32)Width, (f32)Height);

	priv::Resize((f32)Width, (f32)Height);
}

window::~window()
{
	glfwSetWindowUserPointer(m_Window->Window, nullptr);
	glfwDestroyWindow(m_Window->Window);
}

void window::Traverse()
{
	DrawRectangle(0.0f, 0.0f, m_Widget->Size.x, m_Widget->Size.y, MakeColorFromRGB8(255, 0, 255));
	widget::Traverse();
}

void rectangle::Traverse()
{
	DrawRectangle(x.Data, y.Data, w.Data, h.Data, fillColor.Data, radius.Data, borderWidth.Data, borderColor.Data);
	widget::Traverse();
}
}
