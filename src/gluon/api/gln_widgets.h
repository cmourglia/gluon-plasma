#pragma once

#include "gln_api_defs.h"

#include <gluon/core/gln_defines.h>
#include <gluon/core/gln_color.h>

#include <gluon/core/gln_property.h>

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

protected:
	widget_impl* m_Widget;
};

class application;

class GLUON_API_EXPORT window : public widget
{
	friend class application;
	friend bool WindowShouldClose(window* Window);
	friend void SwapBuffers(window* Window);

public:
	explicit window(const char* Title = "");

	virtual ~window();

	void SetTitle(const char* Title);
	void Resize(const i32 Width, const i32 Height);
	void Move(const i32 X, const i32 Y);

protected:
	void Traverse() override final;

protected:
	window_impl* m_Window;
};

class GLUON_API_EXPORT rectangle : public widget
{
public:
	explicit rectangle(widget* Parent = nullptr)
	    : widget(Parent)
	{
	}

	virtual ~rectangle() = default;

protected:
	void Traverse() override final;

public:
	property<f32>   x           = 0.0f;
	property<f32>   y           = 0.0f;
	property<f32>   w           = 30.0f;
	property<f32>   h           = 30.0f;
	property<color> fillColor   = MakeColorFromRGB32(0xFFFF00);
	property<f32>   radius      = 0.0f;
	property<color> borderColor = MakeColorFromRGB32(0);
	property<f32>   borderWidth = 0.0f;
};
}
