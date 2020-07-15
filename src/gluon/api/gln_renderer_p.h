#pragma once

#include <gluon/core/gln_defines.h>

namespace gluon
{
namespace priv
{
	void CreateRenderingContext();
	void DestroyRenderingContext();

	void Resize(f32 Width, f32 Height);
	void SetTextScale(f32 ScaleX, f32 ScaleY);

	void Flush();
}
}
