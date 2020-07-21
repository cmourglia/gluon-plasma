#pragma once

#include "gln_api_defs.h"

#include <gluon/core/gln_defines.h>
#include <gluon/api/gln_inputs.h>

namespace gluon
{
struct application_impl;
class window;

class GLUON_API_EXPORT application
{
	friend void RegisterWindow(window* Window);

public:
	static application* Get();

	application();
	virtual ~application();

	void SetWindowTitle(const char*);
	void SetSize(i32 Width, i32 Height);

	void* GetNativeHandle() const;

	i32 Run();

	virtual void OnResize(i32 NewWidth, i32 NewHeight) {}

	virtual void OnMouseMove(f32 X, f32 Y) {}
	virtual void OnMouseEvent(input_mouse_buttons Button, input_actions Action, input_mods Mods) {}
	virtual void OnKeyEvent(input_keys Key, input_actions Action, input_mods Mods) {}
	virtual void OnCharInput(u32 Codepoint) {}

	virtual void OnUpdate() {}

	void Exit();

	vec2i GetSize() const;

private:
	static application* s_Application;

	application_impl* m_Impl;
};
}
