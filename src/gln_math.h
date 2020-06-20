#pragma once

#include "gln_defines.h"

#include <math.h>

namespace gln
{
constexpr f32 PI   = 3.14159265359f;
constexpr f32 PI_2 = PI / 2.0f;
constexpr f32 PI_3 = PI / 3.0f;
constexpr f32 PI_4 = PI / 4.0f;
constexpr f32 PI_8 = PI / 8.0f;
constexpr f32 TAU  = 2.0f * PI;

constexpr f32 DEG_TO_RAD = PI / 180.0f;
constexpr f32 RAD_TO_DEG = 180.0f / PI;

constexpr f32 SQRT_2 = 1.41421356237f;
constexpr f32 E      = 2.71828182846f; // Euler's number
constexpr f32 PHI    = 1.61803398875f; // Golden ratio

inline f32 Min(const f32 x, const f32 y) { return x < y ? x : y; }
inline f32 Max(const f32 x, const f32 y) { return x > y ? x : y; }
inline f32 Clamp(const f32 x, const f32 a, const f32 b) { return Min(Max(x, a), b); }

inline f32 Sine(const f32 x) { return sinf(x); }
inline f32 Cosine(const f32 x) { return cosf(x); }
inline f32 Tangent(const f32 x) { return tanf(x); }
inline f32 ArcSine(const f32 x) { return asinf(x); }
inline f32 ArcCosine(const f32 x) { return acosf(x); }
inline f32 ArcTangent(const f32 x) { return atanf(x); }

inline f32 Pow(const f32 x, const f32 e) { return powf(x, e); }
inline f32 Sqrt(const f32 x) { return sqrtf(x); }
}
