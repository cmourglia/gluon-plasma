#include <loguru.hpp>

#include <glad/glad.h>
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

#include <rapidjson/document.h>

#include <optick.h>

#include <stb_image.h>

#include "timer.cpp"

#include "renderer.h"
#include "gln_interpolate.h"

static bool gShowStats   = false;
static bool gEnableVSync = true;
static i32  gMsaaLevel   = 0;

static uint32_t g_WindowWidth  = 1024;
static uint32_t g_WindowHeight = 768;

// static bgfx::InstanceDataBuffer gInstanceDataBuffer;

// static bgfx::UniformHandle             ParamsHandle          = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle PositionBuffer        = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle FillColorRadiusBuffer = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle BorderColorSizeBuffer = GLUON_INVALID_HANDLE;
// static bgfx::IndirectBufferHandle      IndirectBufferHandle  = GLUON_INVALID_HANDLE;

static void ErrorCallback(i32 Error, const char* Description) { LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description); }

i32   g_ElemCount = 2;
float g_Delta     = 2.0f / g_ElemCount;
float g_Radius    = g_Delta / 2.0f;

eastl::vector<gln::color>      g_Colors;
static gln::rendering_context* g_Context;
eastl::vector<float>           g_Radii;

gln::color GetRandomColor() { return gln::color{(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX}; }

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

static void CharCallback(GLFWwindow* Window, u32 Codepoint)
{
#if 1
	WrittenString += Codepoint;
#else
	if (Codepoint == '+')
	{
		g_ElemCount *= 2;
		g_ElemCount = eastl::min(g_ElemCount, 1024);
	}

	if (Codepoint == '-')
	{
		g_ElemCount /= 2;
		g_ElemCount = eastl::max(1, g_ElemCount);
	}

	SetColors();
	SetRadii();

	g_Delta  = 2.0f / g_ElemCount;
	g_Radius = g_Delta / 2.0f;

	CurrentTime = 0.0f;
#endif
}

static void ResizeCallback(GLFWwindow* Window, i32 Width, i32 Height)
{
	g_WindowWidth  = Width;
	g_WindowHeight = Height;

	Resize(g_Context, (f32)Width, (f32)Height);
}

static void ContentScaleCallback(GLFWwindow* Window, f32 ScaleX, f32 ScaleY) { LOG_F(INFO, "%f %f", ScaleX, ScaleY); }

struct brick
{
	brick(i32 i, i32 j, i32 MaxCount)
	{
		Delay = ((f32)rand() / RAND_MAX) * 0.1f;

		const f32 FracW = (f32)g_WindowWidth / (MaxCount + 1);
		const f32 FracH = (f32)g_WindowHeight / (MaxCount + 1);

		StartPosition = gln::vec2((f32)(i + 1) * FracW, (f32)(j + 1) * FracH - g_WindowHeight);
		EndPosition   = gln::vec2((f32)(i + 1) * FracW, (f32)(j + 1) * FracH);
		StartColor    = GetRandomColor();
		EndColor      = GetRandomColor();
		EndSize       = 1.0f / MaxCount;
		StartSize     = EndSize * 0.5f;
	}

	f32        Delay;
	gln::vec2  StartPosition, EndPosition;
	f32        StartSize, EndSize;
	gln::color StartColor, EndColor;

	void Render(f32 Time, f32 AnimationTime)
	{
		Time -= Delay;
		auto Position = gln::Interpolate(Time, AnimationTime, StartPosition, EndPosition, gln::EaseOutElastic);
		auto Size     = gln::Interpolate(Time, AnimationTime, StartSize, EndSize, gln::EaseOutBounce);
		// auto Color    = gln::Interpolate(Time, AnimationTime / 2, StartColor, EndColor, gln::EaseLinear, gln::ColorSpace_HSV);
		auto Color = EndColor;

		Size = Size * gln::Min((f32)g_WindowWidth, (f32)g_WindowHeight) * 0.5f;
		gln::DrawRectangle(g_Context, Position.x, Position.y, Size, Size, Color);
	}
};

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
	// glfwSwapInterval(0);

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

	// ParamsHandle = bgfx::createUniform("Params", bgfx::UniformType::Vec4, 2);

	// bgfx::VertexLayout VertexLayout;
	// VertexLayout.begin().add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float).end();

	// PositionBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// FillColorRadiusBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// BorderColorSizeBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// IndirectBufferHandle = bgfx::createIndirectBuffer(256);

	SetColors();
	SetRadii();

	const f32 AnimationTime = 2.0f;

	timer Timer;
	Timer.Start();

#if 1
	eastl::vector<double> Times;

	while (!glfwWindowShouldClose(Window))
	{
		OPTICK_FRAME("MainThread");

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
#else
	eastl::vector<brick> Bricks;
	i32                  MaxCount = 25;
	for (i32 i = 0; i < MaxCount; ++i)
	{
		for (i32 j = 0; j < MaxCount; ++j)
		{
			Bricks.emplace_back(i, j, MaxCount);
		}
	}

	eastl::vector<double> Times;

	while (!glfwWindowShouldClose(Window))
	{
		OPTICK_FRAME("MainThread");

		glfwPollEvents();

		const f32 dt = (f32)Timer.DeltaTime();

		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)g_WindowWidth, (float)g_WindowHeight, 0.0f, 0.0f, 100.0f);
		gln::SetCameraInfo(g_Context, glm::value_ptr(ViewMatrix), glm::value_ptr(ProjMatrix));

		// DrawRectangle(&Context, g_WindowWidth / 2.f, 50, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 150, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 250, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 350, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 450, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 550, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 650, 100, 100, GetRandomColor());

		// if (CurrentTime >= AnimationTime)
		// {
		// CurrentTime = 0.0f;
		// std::swap(CurrentSize, CurrentTarget);
		// }

		for (auto&& Brick : Bricks)
		{
			Brick.Render(CurrentTime, AnimationTime);
		}

		CurrentTime += dt;

		// for (i32 i = 0; i < g_ElemCount; ++i)
		// {
		// 	for (i32 j = 0; j < g_ElemCount; ++j)
		// 	{
		// 		const float x = -1.0f + g_Delta * i + g_Radius;
		// 		const float y = -1.0f + g_Delta * j + g_Radius;

		// 		const float FinalX    = (x + 1.f) * 0.5f * g_WindowWidth;
		// 		const float FinalY    = (y + 1.f) * 0.5f * g_WindowHeight;
		// 		const float FinalSize = Size * eastl::min(g_WindowWidth, g_WindowHeight) * 0.5f;
		// 		// const float FinalRadius = FinalSize / g_Radii[i * g_ElemCount + j];
		// 		gln::DrawRectangle(g_Context, FinalX, FinalY, FinalSize * 2, FinalSize, g_Colors[i * g_ElemCount + j]);
		// 	}
		// }

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
#endif

	OPTICK_SHUTDOWN();

	return 0;
}
