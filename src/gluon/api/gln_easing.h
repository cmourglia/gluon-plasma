#pragma once

#include <gluon/core/gln_math.h>

namespace gluon
{
// https://easings.net/

inline f32 EaseLinear(const f32 t) { return t; }

inline f32 EaseInSine(const f32 t) { return 1.0f - Cosine((t * PI) * 0.5f); }
inline f32 EaseOutSine(const f32 t) { return Sine((t * PI) * 0.5f); }
inline f32 EaseInOutSine(const f32 t) { return -(Cosine(PI * t) - 1.0f) * 0.5f; }

inline f32 EaseInQuad(const f32 t) { return t * t; }
inline f32 EaseOutQuad(const f32 t) { return 1.0f - (1.0f - t) * (1.0f - t); }
inline f32 EaseInOutQuad(const f32 t)
{
	const f32 FirstHalf  = 2.0f * t * t;
	const f32 OneMinusT  = -2.0f * t + 2.0f;
	const f32 SecondHalf = 1 - (OneMinusT * OneMinusT) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInCubic(const f32 t) { return t * t * t; }
inline f32 EaseOutCubic(const f32 t)
{
	const f32 OneMinusT = 1.0f - t;
	return 1.0f - OneMinusT * OneMinusT * OneMinusT;
}
inline f32 EaseInOutCubic(const f32 t)
{
	const f32 FirstHalf  = 4.0f * t * t * t;
	const f32 OneMinusT  = -2.0f * t + 2.0f;
	const f32 SecondHalf = 1.0f - (OneMinusT * OneMinusT * OneMinusT) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInQuart(const f32 t)
{
	const f32 TSquared = t * t;
	return TSquared * TSquared;
}
inline f32 EaseOutQuart(const f32 t)
{
	const f32 OneMinusT        = 1.0f - t;
	const f32 OneMinusTSquared = OneMinusT * OneMinusT;
	return 1.0f - OneMinusTSquared * OneMinusTSquared;
}
inline f32 EaseInOutQuart(const f32 t)
{
	const f32 TSquared         = t * t;
	const f32 FirstHalf        = 8.0f * TSquared * TSquared;
	const f32 OneMinusT        = -2.0f * t + 2.0f;
	const f32 OneMinusTSquared = OneMinusT * OneMinusT;
	const f32 SecondHalf       = 1.0f - (OneMinusTSquared * OneMinusTSquared) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInQuint(const f32 t)
{
	const f32 TSquared = t * t;
	return TSquared * TSquared * t;
}
inline f32 EaseOutQuint(const f32 t)
{
	const f32 OneMinusT        = 1.0f - t;
	const f32 OneMinusTSquared = OneMinusT * OneMinusT;
	return 1.0f - OneMinusTSquared * OneMinusTSquared * t;
}
inline f32 EaseInOutQuint(const f32 t)
{
	const f32 TSquared         = t * t;
	const f32 FirstHalf        = 5.0f * TSquared * TSquared * t;
	const f32 OneMinusT        = 2.0f * t + 2.0f;
	const f32 OneMinusTSquared = OneMinusT * OneMinusT;
	const f32 SecondHalf       = 1.0f - (OneMinusTSquared * OneMinusTSquared * OneMinusT) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInExpo(const f32 t) { return t == 0.0f ? 0.0f : Pow(2.0f, 10.f * t - 10.0f); }
inline f32 EaseOutExpo(const f32 t) { return t == 1.0f ? 1.0f : 1.0f - Pow(2.0f, -10.0f * t); }
inline f32 EaseInOutExpo(const f32 t)
{
	const f32 FirstHalf  = Pow(2.0f, 20.0f * t - 10.0f) * 0.5f;
	const f32 SecondHalf = (2.0f - Pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
	return t == 0 ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInCirc(const f32 t) { return 1.0f - Sqrt(1.0f - t * t); }
inline f32 EaseOutCirc(const f32 t) { Sqrt(1.0f - (t - 1.0f) * (t - 1.0f)); }
inline f32 EaseInOutCirc(const f32 t)
{
	const f32 FirstHalf  = (1.0f - Sqrt(1.0f - 4 * t * t)) * 0.5f;
	const f32 TMinusOne  = -2.0f * t + 2.0f;
	const f32 SecondHalf = (Sqrt(1.0f - TMinusOne * TMinusOne) + 1.0f) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInBack(const f32 t)
{
	const f32 a        = 1.70158f;
	const f32 b        = a + 1.0f;
	const f32 TSquared = t * t;
	return b * TSquared * t - a * TSquared;
}
inline f32 EaseOutBack(const f32 t)
{
	const f32 a                = 1.70158f;
	const f32 b                = a + 1.0f;
	const f32 TMinusOne        = t - 1.0f;
	const f32 TMinusOneSquared = TMinusOne * TMinusOne;
	return 1.0f + b * TMinusOne * TMinusOneSquared + a * TMinusOneSquared;
}
inline f32 EaseInOutBack(const f32 t)
{
	const f32 a          = 1.70158f;
	const f32 b          = a * 1.525f;
	const f32 FirstHalf  = ((4.0f * t * t) * ((b + 1.0f) * 2 * t - b)) * 0.5f;
	const f32 TMinusOne  = 2.0f * t - 2.0f;
	const f32 SecondHalf = ((TMinusOne * TMinusOne) * ((b + 1.0f) * TMinusOne + b) + 2.0f) * 0.5f;
	return t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseInElastic(const f32 t)
{
	const f32 a = (2.0f * PI) / 3.0f;
	return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : -Pow(2.0f, 10.0f * t - 10.0f) * Sine((t * 10.0f - 10.75f) * a);
}
inline f32 EaseOutElastic(const f32 t)
{
	const f32 a = (2.0f * PI) / 3.0f;
	return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : Pow(2.0f, -10.0f * t) * Sine((t * 10.0f - 0.75f) * a) + 1.0f;
}
inline f32 EaseInOutElastic(const f32 t)
{
	const f32 a          = (2.0f * PI) / 4.5f;
	const f32 FirstHalf  = -(Pow(2.0f, 20.0f * t - 10.0f) * Sine((20.0f * t - 11.125f) * a)) * 0.5f;
	const f32 SecondHalf = (Pow(2.0f, -20.0f * t + 10.0f) * Sine((20.0f * t - 11.125f) * a)) * 0.5f + 1.0f;
	return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f ? FirstHalf : SecondHalf;
}

inline f32 EaseOutBounce(const f32 t);
inline f32 EaseInBounce(const f32 t) { return 1.0f - EaseOutBounce(1.0f - t); }
inline f32 EaseOutBounce(f32 t)
{
	const f32 a = 7.5625;
	const f32 b = 2.75;
	if (t < 1.0f / b)
	{
		return a * t * t;
	}
	if (t < 2.0f / b)
	{
		t -= (1.5f / b);
		return a * t * t + 0.75f;
	}
	if (t < 2.5f / b)
	{
		t -= (2.25f / b);
		return a * t * t + 0.9375f;
	}
	t -= (2.625f / b);
	return a * t * t + 0.984375f;
}
inline f32 EaseInOutBounce(const f32 t)
{
	return t < 0.5f ? (1.0f - EaseOutBounce(1.0f - 2.0f * t)) * 0.5f : (1.0f + EaseOutBounce(2.0f * t - 1.0f)) * 0.5f;
}
}
