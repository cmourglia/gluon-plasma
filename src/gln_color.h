#pragma once

#include "gln_defines.h"

namespace gln
{
struct color
{
	union {
		struct
		{
			f32 R, G, B;
		};

		struct
		{
			f32 H, S, V;
		};
	};
	f32 A;
};

color MakeColorFromRGB8(u8 Red, u8 Green, u8 Blue, f32 Alpha = 1.0f);
color MakeColorFromRGBA32(u32 RGBA);
color MakeColorFromRGB32(u32 RGB);

color RgbToHsv(const color& Color);
color HsvToRgb(const color& Color);

color operator+(const color& Color1, const color& Color2);
color operator*(const color& Color, const f32 x);
color operator*(const f32 x, const color& Color);

}
