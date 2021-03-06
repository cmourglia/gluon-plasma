#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/algorithm.h>
#include <EASTL/numeric.h>
#include <EASTL/numeric_limits.h>
#include <EASTL/string.h>
#include <EASTL/unordered_map.h>

#include <gluon/core/gln_timer.h>

#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_interpolate.h>
#include <gluon/api/gln_application.h>
#include <gluon/api/gln_widgets.h>

#include <loguru.hpp>

static uint32_t g_WindowWidth  = 1024;
static uint32_t g_WindowHeight = 768;

i32   g_ElemCount = 2;
float g_Delta     = 2.0f / g_ElemCount;
float g_Radius    = g_Delta / 2.0f;

eastl::vector<gluon::color> g_Colors;
eastl::vector<float>        g_Radii;

gluon::color GetRandomColor() { return gluon::color{(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX}; }

void SetColors()
{
	g_Colors.resize(g_ElemCount * g_ElemCount);
	for (auto& Color : g_Colors)
	{
		Color = GetRandomColor();
	}
}

void SetRadii()
{
	auto GetRandomRadius = []() { return ((float)rand() / RAND_MAX) * 3 + 1; };

	g_Radii.resize(g_ElemCount * g_ElemCount);
	for (auto& R : g_Radii)
	{
		R = GetRandomRadius();
	}
}

static f32 CurrentTime = 0.0f;

eastl::u32string WrittenString;

struct brick
{
	brick(i32 i, i32 j, i32 MaxCount)
	{
		Delay = ((f32)rand() / RAND_MAX) * 0.1f;

		const f32 FracW = (f32)g_WindowWidth / (MaxCount + 1);
		const f32 FracH = (f32)g_WindowHeight / (MaxCount + 1);

		StartPosition = gluon::vec2((f32)(i + 1) * FracW, (f32)(j + 1) * FracH - g_WindowHeight);
		EndPosition   = gluon::vec2((f32)(i + 1) * FracW, (f32)(j + 1) * FracH);
		StartColor    = GetRandomColor();
		EndColor      = GetRandomColor();
		EndSize       = 1.0f / MaxCount;
		StartSize     = EndSize * 0.5f;
		Radius        = ((f32)rand() / RAND_MAX);
	}

	f32          Delay;
	gluon::vec2  StartPosition, EndPosition;
	f32          StartSize, EndSize;
	gluon::color StartColor, EndColor;
	f32          Radius;

	void Render(f32 Time, f32 AnimationTime)
	{
		Time -= Delay;
		auto Position = gluon::Interpolate(Time, AnimationTime, StartPosition, EndPosition, gluon::EaseOutElastic);
		auto Size     = gluon::Interpolate(Time, AnimationTime, StartSize, EndSize, gluon::EaseOutBounce);
		// auto Color    = gluon::Interpolate(Time, AnimationTime / 2, StartColor, EndColor, gluon::EaseLinear, gluon::ColorSpace_HSV);
		auto Color = EndColor;

		Size = Size * gluon::Min((f32)g_WindowWidth, (f32)g_WindowHeight) * 0.5f;
		gluon::DrawRectangle(Position.x, Position.y, Size, Size, Color, Radius * Size);
	}
};

// struct application : public gluon::application
// {
// 	application()
// 	    : gluon::application("Hello")
// 	{
// 		auto Size = GetSize();

// 		g_WindowWidth  = Size.x;
// 		g_WindowHeight = Size.y;

// 		SetColors();
// 		SetRadii();

// 		for (i32 i = 0; i < MaxCount; ++i)
// 		{
// 			for (i32 j = 0; j < MaxCount; ++j)
// 			{
// 				Bricks.emplace_back(i, j, MaxCount);
// 			}
// 		}

// 		Timer.Start();
// 	}

// 	virtual void OnUpdate() override final
// 	{
// 		const f32 dt = (f32)Timer.DeltaTime();

// 		for (auto&& Brick : Bricks)
// 		{
// 			Brick.Render(CurrentTime, AnimationTime);
// 		}

// 		CurrentTime += dt;

// 		Times.push_back(dt);
// 		if (Times.size() == 100)
// 		{
// 			const double Sum = eastl::accumulate(Times.begin(), Times.end(), 0.0);
// 			const double Avg = Sum * 1e-2;

// 			char Buffer[512];
// 			snprintf(Buffer, 512, "GLUON RPZ (%lf FPS - %lf ms)", 1.0 / Avg, Avg * 1000);
// 			SetWindowTitle(Buffer);
// 			Times.clear();
// 		}
// 	}

// 	void OnResize(i32 Width, i32 Height) override final
// 	{
// 		g_WindowWidth  = Width;
// 		g_WindowHeight = Height;

// 		Bricks.clear();
// 		for (i32 i = 0; i < MaxCount; ++i)
// 		{
// 			for (i32 j = 0; j < MaxCount; ++j)
// 			{
// 				Bricks.emplace_back(i, j, MaxCount);
// 			}
// 		}
// 	}

// 	void OnKeyEvent(gluon::input_keys Key, gluon::input_actions Action, gluon::input_mods Mods) override final
// 	{
// 		if (Action == gluon::Action_Release)
// 		{
// 			if ((Key == gluon::Key_Equal) && (Mods & gluon::Mod_Shift))
// 			{
// 				g_ElemCount *= 2;
// 				g_ElemCount = eastl::min(g_ElemCount, 1024);
// 			}

// 			if (Key == gluon::Key_Minus)
// 			{
// 				g_ElemCount /= 2;
// 				g_ElemCount = eastl::max(1, g_ElemCount);
// 			}

// 			if (Key == gluon::Key_Escape)
// 			{
// 				Exit();
// 			}

// 			SetColors();
// 			SetRadii();

// 			g_Delta  = 2.0f / g_ElemCount;
// 			g_Radius = g_Delta / 2.0f;

// 			CurrentTime = 0.0f;
// 		}
// 	}

// 	const f32             AnimationTime = 2.0f;
// 	gluon::timer          Timer;
// 	eastl::vector<double> Times;
// 	eastl::vector<brick>  Bricks;
// 	i32                   MaxCount = 25;
// };

i32 main()
{
	srand(42);

	gluon::application App;
	gluon::window      Window("Hello, gluon");

	f32 y = 10;

	// for (i32 i = 0; i < 8; ++i)
	// {
	// 	f32 x = 10;
	// 	for (i32 j = 0; j < 10; ++j)
	// 	{
	// 		auto Rect       = new gluon::rectangle(&Window);
	// 		Rect->x         = x;
	// 		Rect->y         = y;
	// 		Rect->w         = 50;
	// 		Rect->h         = 50;
	// 		Rect->fillColor = gluon::RandomColor();

	// 		x += 81;
	// 	}
	// 	y += 75.7f;
	// }

	gluon::rectangle r1(&Window);
	r1.fillColor = gluon::MakeColorFromRGB8(255, 0, 0);

	gluon::rectangle r2(&Window);
	r2.x         = 100;
	r2.fillColor = gluon::MakeColorFromRGB8(0, 255, 0);
	r2.fillColor = r1.fillColor;
	r2.fillColor.Subscribe([](const gluon::color& c) { LOG_F(INFO, "Color changed: %d %d %d", c.R, c.G, c.B); });

	r1.fillColor = gluon::RandomColor();
	return App.Run();
}
