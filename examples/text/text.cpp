#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/algorithm.h>
#include <EASTL/numeric.h>
#include <EASTL/numeric_limits.h>
#include <EASTL/string.h>
#include <EASTL/unordered_map.h>

#include <gluon/core/gln_timer.h>

#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_application.h>

#include <loguru.hpp>

static uint32_t g_WindowWidth  = 1024;
static uint32_t g_WindowHeight = 768;

static f32 CurrentTime = 0.0f;

eastl::u32string WrittenString;

struct application : public gluon::application
{
	application()
	    : gluon::application("Text example")
	{
		auto Size = GetSize();

		g_WindowWidth  = Size.x;
		g_WindowHeight = Size.y;

		Timer.Start();
	}

	virtual void OnUpdate() override final
	{
		const f32 dt = (f32)Timer.DeltaTime();

		// gluon::DrawRectangle(ctx, x, y, w, h, fill, radis, border, bordercolor);
		f32 x = g_WindowWidth / 2.0f;
		f32 y = g_WindowHeight / 2.0f;

		gluon::SetFont("DIMIS");
		gluon::DrawText(WrittenString.c_str(), 32, 0, y, gluon::MakeColorFromRGB8(0, 0, 0));
		gluon::SetFont("Lamthong");
		gluon::DrawText(eastl::u32string(U"Hello, World !").c_str(), 256, x / 2, y / 2, gluon::MakeColorFromRGB8(0, 0, 0));
		gluon::SetFont("Roboto");
		gluon::DrawText(eastl::u32string(U"Hello, World !").c_str(), 64, x / 2, y + y / 2, gluon::MakeColorFromRGB8(0, 0, 0));

		Times.push_back(dt);
		if (Times.size() == 100)
		{
			const double Sum = eastl::accumulate(Times.begin(), Times.end(), 0.0);
			const double Avg = Sum * 1e-2;

			char Buffer[512];
			snprintf(Buffer, 512, "GLUON RPZ (%lf FPS - %lf ms)", 1.0 / Avg, Avg * 1000);
			SetWindowTitle(Buffer);
			Times.clear();
		}
	}

	virtual void OnCharInput(u32 Codepoint) override final { WrittenString += Codepoint; }

	virtual void OnKeyEvent(gluon::input_keys Key, gluon::input_actions Action, gluon::input_mods Mods) override final
	{
		if (Key == gluon::Key_Escape && Action == gluon::Action_Release)
		{
			Exit();
		}

		if (Action == gluon::Action_Press || Action == gluon::Action_Repeat)
		{
			if (Key == gluon::Key_Backspace)
			{
				if (!WrittenString.empty())
				{
					if (Mods & gluon::Mod_Control)
					{
						auto Position = WrittenString.find_last_of(U" \r\n");
						if (Position != eastl::u32string::npos)
						{
							WrittenString = WrittenString.substr(0, Position);
						}
						else
						{
							WrittenString.clear();
						}
					}
					else
					{
						WrittenString.pop_back();
					}
				}
			}

			if (Key == gluon::Key_Enter || Key == gluon::Key_NumPadEnter)
			{
				WrittenString += U"\n";
			}
		}
	}

	void OnResize(i32 Width, i32 Height) override final
	{
		g_WindowWidth  = Width;
		g_WindowHeight = Height;
	}

	gluon::timer          Timer;
	eastl::vector<double> Times;
};

int main()
{
	application App;
	return App.Run();
}
