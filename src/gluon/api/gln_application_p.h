#pragma once

#include <EASTL/vector.h>

namespace gluon
{
class window;
struct application_impl
{
	eastl::vector<window*> Windows;

	bool ShouldClose() const;
};

void RegisterWindow(window* Window);
}
