#pragma once

#include "gln_color.h"

namespace gln
{
struct rendering_context;

rendering_context* CreateRenderingContext();
void               DestroyRenderingContext(rendering_context* context);

void Resize(rendering_context* Context, f32 Width, f32 Height);
void SetTextScale(rendering_context* Context, f32 ScaleX, f32 ScaleY);

void SetCameraInfo(rendering_context* Context, const f32* ViewMatrix, const f32* ProjMatrix);

/**
 * @param X X position of the center of the rectangle
 * @param Y Y position of the center of the rectangle
 */
void DrawRectangle(rendering_context* Context,
                   f32                X,
                   f32                Y,
                   f32                Width,
                   f32                Height,
                   color              FillColor,
                   f32                Radius      = 0.0f,
                   f32                BorderWidth = 0.0f,
                   color              BorderColor = {0.0f, 0.0f, 0.0f, 1.0f});

void SetFont(rendering_context* Context, const char* FontName);
void DrawText(rendering_context* Context, const char32_t* Text, f32 PixelSize, f32 X, f32 Y, color FillColor);

void Flush(rendering_context* Context);
}
