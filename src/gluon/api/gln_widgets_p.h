#pragma once

#include <gluon/core/gln_defines.h>

#include <GLFW/glfw3.h>

#include <EASTL/vector.h>

namespace gluon
{
class widget;

struct widget_impl
{
	void ClearChildren();

	widget*                Parent;
	eastl::vector<widget*> Children;

	vec2 Size;
	vec2 Origin;

	bool    IsDeletingChildren = false;
	widget* ChildBeingDeleted  = nullptr;
};

struct window_impl
{
	GLFWwindow* Window;
};

bool WindowShouldClose(window* Window);
void SwapBuffer(window* Window);
}
