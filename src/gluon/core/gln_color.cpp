#include "gln_color.h"

#include "gln_math.h"

namespace gluon
{

color MakeColorFromRGB8(u8 Red, u8 Green, u8 Blue, f32 Alpha)
{
	return {(f32)Red / 255.0f, (f32)Green / 255.0f, (f32)Blue / 255.0f, Alpha};
}

color MakeColorFromRGBA32(u32 RGBA)
{
	const u8 R = (RGBA >> 24) & 0xFF;
	const u8 G = (RGBA >> 16) & 0xFF;
	const u8 B = (RGBA >> 8) & 0xFF;
	const u8 A = (RGBA >> 0) & 0xFF;
	return MakeColorFromRGB8(R, G, B, (f32)A / 255.0f);
}

color MakeColorFromRGB32(u32 RGB)
{
	const u8 R = (RGB >> 16) & 0xFF;
	const u8 G = (RGB >> 8) & 0xFF;
	const u8 B = (RGB >> 0) & 0xFF;
	return MakeColorFromRGB8(R, G, B, 1.0f);
}

color RgbToHsv(const color& Color)
{
	const f32 MinV = Min(Min(Color.R, Color.G), Color.B);
	const f32 MaxV = Max(Max(Color.R, Color.G), Color.B);

	const f32 V     = MaxV;
	const f32 Delta = MaxV - MinV;

	if (abs(Delta) < 1e-5 || abs(MaxV) < 1e-5)
	{
		return {0.0f, 0.0f, V, Color.A};
	}

	const f32 S = Delta / MaxV;

	f32       H        = 0.0f;
	const f32 InvDelta = 1.0f / Delta;

	if (Color.R >= MaxV)
	{
		H = (Color.G - Color.B) * InvDelta;
	}
	else if (Color.G >= MaxV)
	{
		H = 2.0f + (Color.B - Color.R) * InvDelta;
	}
	else
	{
		H = 4.0f + (Color.R - Color.G) * InvDelta;
	}

	H *= 60.0f;
	if (H < 0.0f)
	{
		H += 360.0f;
	}

	return {H, S, V, Color.A};
}

color HsvToRgb(const color& Color)
{
	if (abs(Color.S) < 1e-5)
	{
		return {Color.V, Color.V, Color.V, Color.A};
	}

	const f32 HH = (Color.H >= 360.0f ? 0.0f : Color.H) / 60.0f;
	const i32 I  = (i32)HH;
	const f32 FF = HH - (f32)I;
	const f32 P  = Color.V * (1.0f - Color.S);
	const f32 Q  = Color.V * (1.0f - (Color.S * FF));
	const f32 T  = Color.V * (1.0f - (Color.S * (1.0f - FF)));

	switch (I)
	{
		case 0:
			return {Color.V, T, P, Color.A};
		case 1:
			return {Q, Color.V, P, Color.A};
		case 2:
			return {P, Color.V, T, Color.A};
		case 3:
			return {P, Q, Color.V, Color.A};
		case 4:
			return {T, P, Color.V, Color.A};
		case 5:
			return {Color.V, P, Q, Color.A};
	}

	return {0.0f, 0.0f, 0.0f, Color.A};
}

color operator+(const color& C1, const color& C2) { return {C1.R + C2.R, C1.G + C2.G, C1.B + C2.B, C1.A + C2.A}; }
color operator*(const color& Color, const f32 x) { return {Color.R * x, Color.G * x, Color.B * x, Color.A * x}; }
color operator*(const f32 x, const color& Color) { return {Color.R * x, Color.G * x, Color.B * x, Color.A * x}; }

}
