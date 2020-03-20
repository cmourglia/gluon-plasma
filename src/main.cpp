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

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <optick.h>

#include "timer.cpp"

#include "renderer.h"

static bool gShowStats   = false;
static bool gEnableVSync = true;
static int  gMsaaLevel   = 0;

static uint32_t g_WindowWidth  = 1024;
static uint32_t g_WindowHeight = 768;

// static bgfx::InstanceDataBuffer gInstanceDataBuffer;

// static bgfx::UniformHandle             ParamsHandle          = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle PositionBuffer        = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle FillColorRadiusBuffer = GLUON_INVALID_HANDLE;
// static bgfx::DynamicVertexBufferHandle BorderColorSizeBuffer = GLUON_INVALID_HANDLE;
// static bgfx::IndirectBufferHandle      IndirectBufferHandle  = GLUON_INVALID_HANDLE;

static void ErrorCallback(int Error, const char* Description)
{
	LOG_F(ERROR, "GLFW Error %d: %s\n", Error, Description);
}

int   g_ElemCount = 2;
float g_Delta     = 2.0f / g_ElemCount;
float g_Radius    = g_Delta / 2.0f;

eastl::vector<color>      g_Colors;
static rendering_context* g_Context;

inline color GetRandomColor()
{
	return color{(float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
}

void SetColors()
{
	g_Colors.resize(g_ElemCount * g_ElemCount);
	for (auto& Color : g_Colors)
	{
		Color = GetRandomColor();
	}
}

static void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
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
}

static void CharCallback(GLFWwindow* Window, unsigned int Codepoint)
{
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

	g_Delta  = 2.0f / g_ElemCount;
	g_Radius = g_Delta / 2.0f;
}

static void ResizeCallback(GLFWwindow* Window, int Width, int Height)
{
	g_WindowWidth  = Width;
	g_WindowHeight = Height;

	Resize(g_Context, Width, Height);
}

int main()
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

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	GLFWwindow* Window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "GLUON RPZ", nullptr, nullptr);

	if (!Window)
	{
		LOG_F(FATAL, "Cannot create GLFW Window");
	}

	glfwMakeContextCurrent(Window);

	glfwSetKeyCallback(Window, KeyCallback);
	glfwSetCharCallback(Window, CharCallback);
	glfwSetWindowSizeCallback(Window, ResizeCallback);

	// Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
	// Most graphics APIs must be used on the same thread that created the window.

	int Width, Height;
	glfwGetWindowSize(Window, &Width, &Height);
	g_WindowWidth  = Width;
	g_WindowHeight = Height;

	g_Context = CreateRenderingContext();
	Resize(g_Context, Width, Height);

	// ParamsHandle = bgfx::createUniform("Params", bgfx::UniformType::Vec4, 2);

	// bgfx::VertexLayout VertexLayout;
	// VertexLayout.begin().add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float).end();

	// PositionBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// FillColorRadiusBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// BorderColorSizeBuffer = bgfx::createDynamicVertexBuffer(1 << 15, VertexLayout, BGFX_BUFFER_COMPUTE_READ);
	// IndirectBufferHandle = bgfx::createIndirectBuffer(256);

	SetColors();

	timer Timer;
	Timer.Start();

	eastl::vector<double> Times;

	while (!glfwWindowShouldClose(Window))
	{
		OPTICK_FRAME("MainThread");

		glfwPollEvents();

		const auto ViewMatrix = glm::mat4(1.0f);
		const auto ProjMatrix = glm::orthoLH_ZO(0.0f, (float)g_WindowWidth, (float)g_WindowHeight, 0.0f, 0.0f, 100.0f);
		SetCameraInfo(g_Context, glm::value_ptr(ViewMatrix), glm::value_ptr(ProjMatrix));

		// DrawRectangle(&Context, g_WindowWidth / 2.f, 50, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 150, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 250, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 350, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 450, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 550, 100, 100, GetRandomColor());
		// DrawRectangle(&Context, g_WindowWidth / 2.f, 650, 100, 100, GetRandomColor());

		for (int i = 0; i < g_ElemCount; ++i)
		{
			for (int j = 0; j < g_ElemCount; ++j)
			{
				const float x = -1.0f + g_Delta * i + g_Radius;
				const float y = -1.0f + g_Delta * j + g_Radius;

				const float FinalX      = (x + 1.f) * 0.5f * g_WindowWidth;
				const float FinalY      = (y + 1.f) * 0.5f * g_WindowHeight;
				const float FinalRadius = g_Radius * eastl::min(g_WindowWidth, g_WindowHeight) * 0.5f;

				DrawRectangle(g_Context, FinalX, FinalY, FinalRadius, FinalRadius, g_Colors[i * g_ElemCount + j]);
			}
		}

		Flush(g_Context);

		// Debug
		glfwSwapBuffers(Window);

		Times.push_back(Timer.DeltaTime());
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

	OPTICK_SHUTDOWN();

	return 0;
}
