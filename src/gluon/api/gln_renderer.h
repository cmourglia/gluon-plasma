#pragma once

#include <gluon/api/gln_api_defs.h>

#include <gluon/core/gln_color.h>

namespace gluon
{
struct rendering_context;

GLUON_API_EXPORT rendering_context* CreateRenderingContext();
GLUON_API_EXPORT void               DestroyRenderingContext(rendering_context* context);

GLUON_API_EXPORT void Resize(rendering_context* Context, f32 Width, f32 Height);
GLUON_API_EXPORT void SetTextScale(rendering_context* Context, f32 ScaleX, f32 ScaleY);

GLUON_API_EXPORT void SetCameraInfo(rendering_context* Context, const f32* ViewMatrix, const f32* ProjMatrix);

/**
 * @param X X position of the center of the rectangle
 * @param Y Y position of the center of the rectangle
 */
GLUON_API_EXPORT void DrawRectangle(rendering_context* Context,
                                    f32                X,
                                    f32                Y,
                                    f32                Width,
                                    f32                Height,
                                    color              FillColor,
                                    f32                Radius      = 0.0f,
                                    f32                BorderWidth = 0.0f,
                                    color              BorderColor = {0.0f, 0.0f, 0.0f, 1.0f});

GLUON_API_EXPORT void SetFont(rendering_context* Context, const char* FontName);
GLUON_API_EXPORT void DrawText(rendering_context* Context, const char32_t* Text, f32 PixelSize, f32 X, f32 Y, color FillColor);

GLUON_API_EXPORT void Flush(rendering_context* Context);
}
