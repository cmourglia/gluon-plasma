#include "gvulkan.h"
#include "gswapchain.h"
#include "gresources.h"
#include "gshader.h"
#include "gmesh_loader.h"

#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <loguru.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <algorithm>

static void CursorPosCallback(GLFWwindow* Window, f64 X, f64 Y);
static void MouseButtonCallback(GLFWwindow* Window, i32 Button, i32 Action, i32 Mods);
static void KeyCallback(GLFWwindow* Window, i32 Key, i32 Scancode, i32 Action, i32 Mods);

class GHelloTriangleApplication
{
public:
	void Run()
	{
		Init();
		MainLoop();
		Cleanup();
	}

private:
	void Init()
	{
		InitWindow();
		LoadMesh("Assets/Meshes/Cubes/Cubes.fbx", &Model);
		InitVulkan();
	}

	void MainLoop()
	{
		VkSemaphore AcquireSemaphore = CreateSemaphore(Device);
		VkSemaphore ReleaseSemaphore = CreateSemaphore(Device);

		while (!glfwWindowShouldClose(Window))
		{
			glfwPollEvents();

			u32 ImageIndex = 0;
			VK_CHECK(vkAcquireNextImageKHR(Device, Swapchain.Swapchain, ~0ull, AcquireSemaphore, VK_NULL_HANDLE, &ImageIndex));

			VK_CHECK(vkResetCommandPool(Device, CommandPool, 0));

			VkCommandBufferBeginInfo CommandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
			CommandBufferBeginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			VK_CHECK(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo));

			VkImageMemoryBarrier RenderBeginBarrier = ImageBarrier(Swapchain.Images[ImageIndex],
			                                                       0,
			                                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			                                                       VK_IMAGE_LAYOUT_UNDEFINED,
			                                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			vkCmdPipelineBarrier(CommandBuffer,
			                     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			                     VK_DEPENDENCY_BY_REGION_BIT,
			                     0,
			                     nullptr,
			                     0,
			                     nullptr,
			                     1,
			                     &RenderBeginBarrier);

			VkClearColorValue Color      = {1, 0, 1, 1};
			VkClearValue      ClearColor = {Color};

			VkRenderPassBeginInfo PassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			PassBeginInfo.renderPass            = RenderPass;
			PassBeginInfo.framebuffer           = Swapchain.Frameubffers[ImageIndex];
			PassBeginInfo.renderArea.offset     = {0, 0};
			PassBeginInfo.renderArea.extent     = Swapchain.Extent;
			PassBeginInfo.clearValueCount       = 1;
			PassBeginInfo.pClearValues          = &ClearColor;

			vkCmdBeginRenderPass(CommandBuffer, &PassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport Viewport = {0, (f32)Swapchain.Extent.height, (f32)Swapchain.Extent.width, -(f32)Swapchain.Extent.height, 0, 1};
			VkRect2D   Scissor  = {{0, 0}, {Swapchain.Extent.width, Swapchain.Extent.height}};

			vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);
			vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

			vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

			for (u32 MeshIndex = 0; MeshIndex < Model.Meshes.size(); ++MeshIndex)
			{
				const GMesh& Mesh = Model.Meshes[MeshIndex];

				glm::mat4 Data[3];
				Data[0] = Mesh.ModelMatrix;
				Data[1] = glm::lookAt(glm::vec3(5.0f, 2.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				Data[2] = glm::perspective(glm::quarter_pi<f32>(), 1024.f / 768.f, 0.1f, 100.0f);

				memcpy(TransformUniformBuffers[MeshIndex].Data, Data, 3 * sizeof(glm::mat4));

				VkWriteDescriptorSet WriteDescriptorSets[2] = {};

				GDescriptorInfo Descriptors[] = {{VertexBuffer.Buffer}, {TransformUniformBuffers[MeshIndex].Buffer}};
				vkCmdPushDescriptorSetWithTemplateKHR(CommandBuffer, MeshProgram.UpdateTemplate, MeshProgram.Layout, 0, Descriptors);

				vkCmdDrawIndexed(CommandBuffer, Mesh.IndexCount, 1, Mesh.BaseIndex, Mesh.BaseVertex, 0);
			}

			vkCmdEndRenderPass(CommandBuffer);

			VkImageMemoryBarrier RenderEndBarrier = ImageBarrier(Swapchain.Images[ImageIndex],
			                                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			                                                     0,
			                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			                                                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			vkCmdPipelineBarrier(CommandBuffer,
			                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			                     VK_DEPENDENCY_BY_REGION_BIT,
			                     0,
			                     nullptr,
			                     0,
			                     nullptr,
			                     1,
			                     &RenderEndBarrier);

			VK_CHECK(vkEndCommandBuffer(CommandBuffer));

			VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo SubmitInfo         = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
			SubmitInfo.waitSemaphoreCount   = 1;
			SubmitInfo.pWaitSemaphores      = &AcquireSemaphore;
			SubmitInfo.pWaitDstStageMask    = &PipelineStageFlags;
			SubmitInfo.commandBufferCount   = 1;
			SubmitInfo.pCommandBuffers      = &CommandBuffer;
			SubmitInfo.signalSemaphoreCount = 1;
			SubmitInfo.pSignalSemaphores    = &ReleaseSemaphore;

			VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));

			VkPresentInfoKHR PresentInfo   = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
			PresentInfo.swapchainCount     = 1;
			PresentInfo.pSwapchains        = &Swapchain.Swapchain;
			PresentInfo.pImageIndices      = &ImageIndex;
			PresentInfo.waitSemaphoreCount = 1;
			PresentInfo.pWaitSemaphores    = &ReleaseSemaphore;

			VK_CHECK(vkQueuePresentKHR(PresentQueue, &PresentInfo));

			VK_CHECK(vkDeviceWaitIdle(Device));
		}

		VK_CHECK(vkDeviceWaitIdle(Device));

		vkDestroySemaphore(Device, ReleaseSemaphore, nullptr);
		vkDestroySemaphore(Device, AcquireSemaphore, nullptr);
	}

	void InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		Window = glfwCreateWindow(1024, 768, "Gluon", nullptr, nullptr);

		glfwSetWindowUserPointer(Window, this);
		glfwSetCursorPosCallback(Window, CursorPosCallback);
		glfwSetMouseButtonCallback(Window, MouseButtonCallback);
		glfwSetKeyCallback(Window, KeyCallback);
	}

	void InitVulkan()
	{
		VK_CHECK(volkInitialize());

		auto Extensions = GetRequiredExtensions();
		auto Layers     = GetRequiredLayers();

		Instance = CreateInstance((u32)Extensions.size(), Extensions.data(), (u32)Layers.size(), Layers.data());
		assert(Instance);

		volkLoadInstance(Instance);

#ifdef _DEBUG
		DebugMessenger = SetupDebugMessenger(Instance);
#endif

		VK_CHECK(glfwCreateWindowSurface(Instance, Window, nullptr, &Surface));
		assert(Surface);

		PhysicalDevice = PickPhysicalDevice(Instance, Surface, &QueueIndices);
		assert(PhysicalDevice);

		const char* DeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};

		Device = CreateLogicalDevice(PhysicalDevice, QueueIndices, ARRAY_SIZE(DeviceExtensions), DeviceExtensions);
		assert(Device);

		vkGetDeviceQueue(Device, QueueIndices.GraphicsFamily.Value(), 0, &GraphicsQueue);
		vkGetDeviceQueue(Device, QueueIndices.PresentFamily.Value(), 0, &PresentQueue);
		vkGetDeviceQueue(Device, QueueIndices.ComputeFamily.Value(), 0, &ComputeQueue);

		Swapchain = CreateSwapchain(PhysicalDevice, Device, Surface, 1024, 768, QueueIndices);

		GShader VertexShader   = LoadShader(Device, "Assets/Shaders/Bin/triangle.vert.spv");
		GShader FragmentShader = LoadShader(Device, "Assets/Shaders/Bin/triangle.frag.spv");

		RenderPass = CreateRenderPass(Device, Swapchain.Format);

		MeshProgram = CreateProgram(Device, VK_PIPELINE_BIND_POINT_GRAPHICS, {&VertexShader, &FragmentShader});

		Pipeline = CreateGraphicsPipeline(Device, RenderPass, MeshProgram.Layout, {&VertexShader, &FragmentShader});

		DestroyShader(Device, &VertexShader);
		DestroyShader(Device, &FragmentShader);

		CreateSwapchainFramebuffers(Device, RenderPass, &Swapchain);

		CommandPool = CreateCommandPool(Device, QueueIndices.GraphicsFamily.Value());

		VkCommandBufferAllocateInfo CommandBufferAI = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		CommandBufferAI.commandPool                 = CommandPool;
		CommandBufferAI.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAI.commandBufferCount          = 1;
		VK_CHECK(vkAllocateCommandBuffers(Device, &CommandBufferAI, &CommandBuffer));

		VkPhysicalDeviceMemoryProperties MemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);

		VertexBuffer = CreateBuffer(Device, MemoryProperties, Model.Vertices.size() * sizeof(GVertex), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		IndexBuffer  = CreateBuffer(Device, MemoryProperties, Model.Indices.size() * sizeof(u32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		TransformUniformBuffers.resize(Model.Meshes.size());
		for (auto&& Buffer : TransformUniformBuffers)
		{
			Buffer = CreateBuffer(Device, MemoryProperties, 3 * sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}

		memcpy(VertexBuffer.Data, Model.Vertices.data(), Model.Vertices.size() * sizeof(GVertex));
		memcpy(IndexBuffer.Data, Model.Indices.data(), Model.Indices.size() * sizeof(u32));
	}

	void Cleanup()
	{
		for (auto&& Buffer : TransformUniformBuffers)
		{
			DestroyBuffer(Device, &Buffer);
		}
		DestroyBuffer(Device, &VertexBuffer);
		DestroyBuffer(Device, &IndexBuffer);

		vkDestroyCommandPool(Device, CommandPool, nullptr);

		vkDestroyPipeline(Device, Pipeline, nullptr);
		DestroyProgram(Device, &MeshProgram);
		vkDestroyRenderPass(Device, RenderPass, nullptr);

		DestroySwapchain(Device, &Swapchain);
		vkDestroyDevice(Device, nullptr);
#ifdef _DEBUG
		DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
#endif
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		vkDestroyInstance(Instance, nullptr);

		glfwDestroyWindow(Window);
		glfwTerminate();
	}

	std::vector<const char*> GetRequiredExtensions()
	{
		u32          GLFWExtensionCount = 0;
		const char** GLFWExtensions     = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

		std::vector<const char*> Extensions(GLFWExtensions, GLFWExtensions + GLFWExtensionCount);

#ifdef _DEBUG
		Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return Extensions;
	}

	std::vector<const char*> GetRequiredLayers()
	{
#ifdef _DEBUG
		static const char* ValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
		if (!CheckValidationLayerSupport(1, ValidationLayers))
		{
			exit(1);
		}

		return std::vector<const char*>(ValidationLayers, ValidationLayers + 1);
#else
		return std::vector<const char*>();
#endif
	}

private:
	GLFWwindow* Window;

	GModel Model;

	VkInstance               Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkPhysicalDevice         PhysicalDevice;
	VkDevice                 Device;
	GQueueFamilyIndices      QueueIndices;
	VkSurfaceKHR             Surface;
	GSwapchain               Swapchain;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;
	VkQueue ComputeQueue;

	VkPipeline   Pipeline;
	VkRenderPass RenderPass;
	GProgram     MeshProgram;

	VkCommandPool   CommandPool;
	VkCommandBuffer CommandBuffer;

	GBuffer              VertexBuffer;
	GBuffer              IndexBuffer;
	std::vector<GBuffer> TransformUniformBuffers;

public:
	void MouseMoved(f64 X, f64 Y)
	{
		f64 DeltaX = m_LastMouseX - X;
		f64 DeltaY = m_LastMouseY - Y;

		if (m_LeftButtonPressed)
		{
		}

		m_LastMouseX = X;
		m_LastMouseY = Y;
	}

	void MouseButtonPressed(i32 Button, i32 Mods)
	{
		if (GLFW_MOUSE_BUTTON_LEFT == Button)
		{
			m_LeftButtonPressed = true;
		}
	}

	void MouseButtonReleased(i32 Button, i32 Mods)
	{
		if (GLFW_MOUSE_BUTTON_LEFT == Button)
		{
			m_LeftButtonPressed = false;
		}
	}

	void KeyPressed(i32 Key, i32 Scancode, i32 Mods) {}
	void KeyReleased(i32 Key, i32 Scancode, i32 Mods) {}

private:
	bool m_LeftButtonPressed;
	f64  m_LastMouseX, m_LastMouseY;
};

int main()
{
	GHelloTriangleApplication App;
	App.Run();

	return 0;
}

static void CursorPosCallback(GLFWwindow* Window, f64 X, f64 Y)
{
	GHelloTriangleApplication* App = (GHelloTriangleApplication*)glfwGetWindowUserPointer(Window);
	App->MouseMoved(X, Y);
}

static void MouseButtonCallback(GLFWwindow* Window, i32 Button, i32 Action, i32 Mods)
{
	GHelloTriangleApplication* App = (GHelloTriangleApplication*)glfwGetWindowUserPointer(Window);
	if (GLFW_PRESS == Action)
	{
		App->MouseButtonPressed(Button, Mods);
	}
	else if (GLFW_RELEASE == Action)
	{
		App->MouseButtonReleased(Button, Mods);
	}
}

static void KeyCallback(GLFWwindow* Window, i32 Key, i32 Scancode, i32 Action, i32 Mods)
{
	GHelloTriangleApplication* App = (GHelloTriangleApplication*)glfwGetWindowUserPointer(Window);
	if (GLFW_PRESS == Action)
	{
		App->KeyPressed(Key, Scancode, Mods);
	}
	else if (GLFW_RELEASE == Action)
	{
		App->KeyReleased(Key, Scancode, Mods);
	}
}
