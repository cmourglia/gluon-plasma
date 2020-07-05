#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <eastl/array.h>
#include <eastl/vector.h>
#include <eastl/algorithm.h>
#include <eastl/numeric.h>
#include <eastl/numeric_limits.h>
#include <eastl/string.h>
#include <eastl/unordered_map.h>

#include <gluon/core/gln_timer.h>

#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_interpolate.h>

#include <loguru.hpp>

static bool gShowStats   = false;
static bool gEnableVSync = true;
static i32  gMsaaLevel   = 0;

static uint32_t g_WindowWidth  = 1024;
static uint32_t g_WindowHeight = 768;

static void ErrorCallback(i32 Error, const char* Description) { LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description); }

i32   g_ElemCount = 2;
float g_Delta     = 2.0f / g_ElemCount;
float g_Radius    = g_Delta / 2.0f;

static gln::rendering_context* g_Context;

static f32 CurrentTime = 0.0f;

eastl::u32string WrittenString;

static void KeyCallback(GLFWwindow* Window, i32 Key, i32 Scancode, i32 Action, i32 Mods)
{
	if (Key == GLFW_KEY_F1 && Action == GLFW_RELEASE)
	{
		gShowStats = !gShowStats;
	}

	if (Key == GLFW_KEY_F2 && Action == GLFW_RELEASE)
	{
		gEnableVSync = !gEnableVSync;
		glfwSwapInterval(gEnableVSync ? 1 : 0);
	}

	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(Window, true);
	}

	if (Key == GLFW_KEY_BACKSPACE && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		if (!WrittenString.empty())
		{
			if (Mods &= GLFW_MOD_CONTROL)
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

	if ((Key == GLFW_KEY_ENTER || Key == GLFW_KEY_KP_ENTER) && (Action == GLFW_PRESS || Action == GLFW_REPEAT))
	{
		WrittenString += U"\n";
	}
}

static void CharCallback(GLFWwindow* Window, u32 Codepoint) { WrittenString += Codepoint; }

static void ResizeCallback(GLFWwindow* Window, i32 Width, i32 Height)
{
	g_WindowWidth  = Width;
	g_WindowHeight = Height;

	Resize(g_Context, (f32)Width, (f32)Height);
}

static void ContentScaleCallback(GLFWwindow* Window, f32 ScaleX, f32 ScaleY) { LOG_F(INFO, "%f %f", ScaleX, ScaleY); }

i32 main()
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit())
	{
		LOG_F(FATAL, "Cannot initialize GLFW");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	GLFWwindow* Window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "GLUON RPZ", nullptr, nullptr);

	if (!Window)
	{
		LOG_F(FATAL, "Cannot create GLFW Window");
	}

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(0);

	glfwSetKeyCallback(Window, KeyCallback);
	glfwSetCharCallback(Window, CharCallback);
	glfwSetWindowSizeCallback(Window, ResizeCallback);
	glfwSetWindowContentScaleCallback(Window, ContentScaleCallback);

	// Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
	// Most graphics APIs must be used on the same thread that created the window.

	i32 Width, Height;
	glfwGetWindowSize(Window, &Width, &Height);
	g_WindowWidth  = Width;
	g_WindowHeight = Height;

	f32 ScaleX, ScaleY;
	glfwGetWindowContentScale(Window, &ScaleX, &ScaleY);

	g_Context = gln::CreateRenderingContext();
	gln::Resize(g_Context, Width, Height);
	gln::SetTextScale(g_Context, ScaleX, ScaleY);

	const f32 AnimationTime = 2.0f;

	timer Timer;
	Timer.Start();

	eastl::vector<double> Times;

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();

		const f32 dt = (f32)Timer.DeltaTime();

		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)g_WindowWidth, (float)g_WindowHeight, 0.0f, 0.0f, 100.0f);
		gln::SetCameraInfo(g_Context, glm::value_ptr(ViewMatrix), glm::value_ptr(ProjMatrix));

		// gln::DrawRectangle(ctx, x, y, w, h, fill, radis, border, bordercolor);
		f32 x = g_WindowWidth / 2.0f;
		f32 y = g_WindowHeight / 2.0f;

		gln::SetFont(g_Context, "DIMIS");
		gln::DrawText(g_Context, WrittenString.c_str(), 32, 0, y, gln::MakeColorFromRGB8(0, 0, 0));
		gln::SetFont(g_Context, "Lamthong");
		gln::DrawText(g_Context, eastl::u32string(U"Hello, World !").c_str(), 256, x / 2, y / 2, gln::MakeColorFromRGB8(0, 0, 0));
		gln::SetFont(g_Context, "Roboto");
		gln::DrawText(g_Context, eastl::u32string(U"Hello, World !").c_str(), 64, x / 2, y + y / 2, gln::MakeColorFromRGB8(0, 0, 0));

		CurrentTime += dt;

		gln::Flush(g_Context);

		// Debug
		glfwSwapBuffers(Window);

		Times.push_back(dt);
		if (Times.size() == 100)
		{
			const double Sum = eastl::accumulate(Times.begin(), Times.end(), 0.0);
			const double Avg = Sum * 1e-2;

			char Buffer[512];
			snprintf(Buffer, 512, "GLUON RPZ (%lf FPS - %lf ms)", 1.0 / Avg, Avg * 1000);
			glfwSetWindowTitle(Window, Buffer);
			Times.clear();
		}
	}

	return 0;
}
