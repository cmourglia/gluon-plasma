#include <gluon/api/gln_application.h>

#include <EASTL/vector.h>
#include <EASTL/numeric.h>

#include <gluon/core/gln_timer.h>

struct application : public gluon::application
{
	application()
	    : gluon::application("Hello, World")
	{
		Timer.Start();
	}

	void OnKeyEvent(gluon::input_keys Key, gluon::input_actions Action, gluon::input_mods Mods) override final
	{
		if (Key == gluon::Key_Escape && Action == gluon::Action_Release)
		{
			Exit();
		}
	}

	void OnUpdate() override final
	{
		const f32 dt = (f32)Timer.DeltaTime();

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

	gluon::timer          Timer;
	eastl::vector<double> Times;
};

int main()
{
	application App;
	return App.Run();
}
