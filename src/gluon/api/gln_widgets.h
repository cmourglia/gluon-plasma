#pragma once

#include "gln_api_defs.h"

#include <gluon/api/gln_inputs.h>

#include <gluon/core/gln_defines.h>
#include <gluon/core/gln_color.h>
#include <gluon/core/gln_property.h>
#include <gluon/core/gln_signal.h>

struct GLFWwindow;

namespace gluon
{
/**
 * Until the node's parent is not set, its memory is not managed.
 * Once it has been inserted into the hierarchy (via constructor or @see SetParent()),
 * the memory will be managed / owner by the parent.
 * Hence,
 */
struct widget_impl;
struct window_impl;

class GLUON_API_EXPORT widget
{
	friend class application;

public:
	explicit widget(widget* Parent = nullptr);
	virtual ~widget();

	//! Takes the pointer ownership
	void SetParent(widget* Parent);

protected:
	virtual void Traverse();

	virtual bool OnMouseMove(f32 X, f32 Y);
	virtual bool OnMouseEvent(input_mouse_buttons Button, input_actions Action, input_mods Mods);

protected:
	widget_impl* m_Widget;
};

class application;

class GLUON_API_EXPORT window : public widget
{
	friend class application;
	friend bool WindowShouldClose(window* Window);
	friend void SwapBuffers(window* Window);
	// friend static void ResizeCallback(GLFWwindow* Window, i32 Width, i32 Height);
	friend static void CursorPosCallback(GLFWwindow* Window, f64 X, f64 Y);
	friend static void MouseButtonCallback(GLFWwindow* Window, i32 Button, i32 Action, i32 Mods);

public:
	explicit window(const char* Title = "");

	virtual ~window();

	void SetTitle(const char* Title);
	void Resize(const i32 Width, const i32 Height);
	void Move(const i32 X, const i32 Y);

protected:
	void Traverse() override final;

	// virtual bool OnMouseMove(f32 X, f32 Y) override final;
	// virtual bool OnMouseEvent(input_mouse_buttons Button, input_actions Action, input_mods Mods) override final;

protected:
	window_impl* m_Window;
};

class GLUON_API_EXPORT rectangle : public widget
{
public:
	explicit rectangle(widget* Parent = nullptr);

	virtual ~rectangle() = default;

protected:
	void Traverse() override final;

	bool OnMouseMove(f32 X, f32 Y) override final;
	bool OnMouseEvent(input_mouse_buttons Button, input_actions Action, input_mods Mods) override final;

public:
	property<f32>   x           = 0.0f;
	property<f32>   y           = 0.0f;
	property<f32>   w           = 30.0f;
	property<f32>   h           = 30.0f;
	property<color> fillColor   = MakeColorFromRGB32(0xFFFF00);
	property<f32>   radius      = 0.0f;
	property<color> borderColor = MakeColorFromRGB32(0);
	property<f32>   borderWidth = 0.0f;

	property<bool> hovered = false;
	property<bool> pressed = false;

	signal onEnter;
	signal onLeave;
	signal onPress;
	signal onRelease;
	signal onClick;
};
}
