#pragma once

#include <gluon/api/gln_api_defs.h>

#include <gluon/core/gln_color.h>

namespace gluon
{

/**
 * @param X X position of the center of the rectangle
 * @param Y Y position of the center of the rectangle
 */
GLUON_API_EXPORT void DrawRectangle(f32   X,
                                    f32   Y,
                                    f32   Width,
                                    f32   Height,
                                    color FillColor,
                                    f32   Radius      = 0.0f,
                                    f32   BorderWidth = 0.0f,
                                    color BorderColor = {0.0f, 0.0f, 0.0f, 1.0f});

GLUON_API_EXPORT void SetFont(const char* FontName);
GLUON_API_EXPORT void DrawText(const char32_t* Text, f32 PixelSize, f32 X, f32 Y, color FillColor);
}
