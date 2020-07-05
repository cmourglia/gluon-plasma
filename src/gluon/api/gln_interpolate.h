#pragma once

#include <gluon/api/gln_easing.h>

#include <gluon/core/gln_defines.h>

namespace gln
{

enum easing_function
{
	EasingFunction_Linear = 0,
	EasingFunction_InSine,
	EasingFunction_OutSine,
	EasingFunction_InOutSine,
	EasingFunction_InQuad,
	EasingFunction_OutQuad,
	EasingFunction_InOutQuad,
	EasingFunction_InCubic,
	EasingFunction_OutCubic,
	EasingFunction_InOutCubic,
	EasingFunction_InQuart,
	EasingFunction_OutQuart,
	EasingFunction_InOutQuart,
	EasingFunction_InQuint,
	EasingFunction_OutQuint,
	EasingFunction_InOutQuint,
	EasingFunction_InExpo,
	EasingFunction_OutExpo,
	EasingFunction_InOutExpo,
	EasingFunction_InCirc,
	EasingFunction_OutCirc,
	EasingFunction_InOutCirc,
	EasingFunction_InBack,
	EasingFunction_OutBack,
	EasingFunction_InOutBack,
	EasingFunction_InElastic,
	EasingFunction_OutElastic,
	EasingFunction_InOutElastic,
	EasingFunction_InBounce,
	EasingFunction_OutBounce,
	EasingFunction_InOutBounce,
	EasingFunction_Count,
};

enum color_space
{
	ColorSpace_RGB,
	ColorSpace_HSV,
};

using easing_fn = f32 (*)(const f32);

template <typename T>
inline T Interpolate(const f32 t, const f32 AnimationTime, const T& StartValue, const T& EndValue, easing_fn EasingFunction)
{
	const f32 EasedT = (*EasingFunction)(Clamp(1.0f - ((AnimationTime - t) / AnimationTime), 0.0f, 1.0f));
	T         Result = EndValue * EasedT + StartValue * (1.0f - EasedT);
	return Result;
}

inline color Interpolate(const f32    t,
                         const f32    AnimationTime,
                         const color& StartColor,
                         const color& EndColor,
                         easing_fn    EasingFunction,
                         color_space  ColorSpace)
{
	switch (ColorSpace)
	{
		case ColorSpace_RGB:
		{
			return Interpolate(t, AnimationTime, StartColor, EndColor, EasingFunction);
		}
		break;

		case ColorSpace_HSV:
		{
			const color StartColorHSV = RgbToHsv(StartColor);
			const color EndColorHSV   = RgbToHsv(EndColor);
			const color ColorHSV      = Interpolate(t, AnimationTime, StartColorHSV, EndColorHSV, EasingFunction);
			return HsvToRgb(ColorHSV);
		}
		break;
	}

	return Interpolate(t, AnimationTime, StartColor, EndColor, EasingFunction);
}

}
