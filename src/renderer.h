#pragma once

#include "gln_color.h"

namespace gln {
struct rendering_context;

rendering_context* CreateRenderingContext();
void               DestroyRenderingContext(rendering_context* context);

void Resize(rendering_context* Context, float Width, float Height);
void SetCameraInfo(rendering_context* Context, const float* ViewMatrix, const float* ProjMatrix);

/**
 * @param X X position of the center of the rectangle
 * @param Y Y position of the center of the rectangle
 */
void DrawRectangle(rendering_context* Context,
                   float              X,
                   float              Y,
                   float              Width,
                   float              Height,
                   color              FillColor,
                   float              Radius      = 0.0f,
                   float              BorderWidth = 0.0f,
                   color              BorderColor = {0.0f, 0.0f, 0.0f, 1.0f});

void Flush(rendering_context* Context);
}
