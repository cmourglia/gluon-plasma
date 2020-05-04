#include "gvulkan.h"
#include "gswapchain.h"
#include "gbuffer.h"

#include <glfw/glfw3.h>

#include <glm/glm.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

struct GVertex
{
	f32 Position[2];
	f32 Color[3];
};

struct GMesh
{
	std::vector<GVertex> Vertices;
	std::vector<u16>     Indices;
};

static const GMesh Mesh = {
    {
        {0.0f, -0.5f, 1.0f, 0.0f, 0.0f},
        {0.5f, 0.5f, 0.0f, 1.0f, 0.0f},
        {-0.5f, 0.5f, 0.0f, 0.0f, 1.0f},
    },
    {0, 1, 2},
};

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

			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

			VkDeviceSize DummyOffset = 0;
			vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer.Buffer, &DummyOffset);
			vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(CommandBuffer, (u32)Mesh.Indices.size(), 1, 0, 0, 0);

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

		const char* DeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		Device = CreateLogicalDevice(PhysicalDevice, QueueIndices, 1, DeviceExtensions);
		assert(Device);

		vkGetDeviceQueue(Device, QueueIndices.GraphicsFamily.Value(), 0, &GraphicsQueue);
		vkGetDeviceQueue(Device, QueueIndices.PresentFamily.Value(), 0, &PresentQueue);
		vkGetDeviceQueue(Device, QueueIndices.ComputeFamily.Value(), 0, &ComputeQueue);

		Swapchain = CreateSwapchain(PhysicalDevice, Device, Surface, 1024, 768, QueueIndices);

		CreateGraphicsPipeline();

		CreateSwapchainFramebuffers(Device, RenderPass, &Swapchain);

		CommandPool = CreateCommandPool(Device, QueueIndices.GraphicsFamily.Value());

		VkCommandBufferAllocateInfo CommandBufferAI = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		CommandBufferAI.commandPool                 = CommandPool;
		CommandBufferAI.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAI.commandBufferCount          = 1;
		VK_CHECK(vkAllocateCommandBuffers(Device, &CommandBufferAI, &CommandBuffer));

		VkPhysicalDeviceMemoryProperties MemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);

		VertexBuffer = CreateBuffer(Device, MemoryProperties, Mesh.Vertices.size() * sizeof(GVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		IndexBuffer  = CreateBuffer(Device, MemoryProperties, Mesh.Indices.size() * sizeof(u16), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		memcpy(VertexBuffer.Data, Mesh.Vertices.data(), Mesh.Vertices.size() * sizeof(GVertex));
		memcpy(IndexBuffer.Data, Mesh.Indices.data(), Mesh.Indices.size() * sizeof(u16));
	}

	void Cleanup()
	{
		DestroyBuffer(Device, &VertexBuffer);
		DestroyBuffer(Device, &IndexBuffer);

		vkDestroyCommandPool(Device, CommandPool, nullptr);

		vkDestroyPipeline(Device, Pipeline, nullptr);
		vkDestroyRenderPass(Device, RenderPass, nullptr);
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);

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

	void CreateGraphicsPipeline()
	{
		VkShaderModule VertexShader   = CreateShader(Device, "Assets/Shaders/Bin/triangle.vert.spv");
		VkShaderModule FragmentShader = CreateShader(Device, "Assets/Shaders/Bin/triangle.frag.spv");

		assert(VertexShader != VK_NULL_HANDLE);
		assert(FragmentShader != VK_NULL_HANDLE);

		VkPipelineShaderStageCreateInfo ShaderStageCIs[2] = {};

		ShaderStageCIs[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStageCIs[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		ShaderStageCIs[0].module = VertexShader;
		ShaderStageCIs[0].pName  = "main";

		ShaderStageCIs[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStageCIs[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		ShaderStageCIs[1].module = FragmentShader;
		ShaderStageCIs[1].pName  = "main";

		VkVertexInputBindingDescription VertexInputBindingDescriptions[1] = {};
		VertexInputBindingDescriptions[0].binding                         = 0;
		VertexInputBindingDescriptions[0].stride                          = sizeof(GVertex);
		VertexInputBindingDescriptions[0].inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription VertexInputAttributeDescriptions[2] = {};
		VertexInputAttributeDescriptions[0].binding                           = 0;
		VertexInputAttributeDescriptions[0].format                            = VK_FORMAT_R32G32_SFLOAT;
		VertexInputAttributeDescriptions[0].location                          = 0;
		VertexInputAttributeDescriptions[0].offset                            = offsetof(GVertex, Position);
		VertexInputAttributeDescriptions[1].binding                           = 0;
		VertexInputAttributeDescriptions[1].format                            = VK_FORMAT_R32G32B32_SFLOAT;
		VertexInputAttributeDescriptions[1].location                          = 1;
		VertexInputAttributeDescriptions[1].offset                            = offsetof(GVertex, Color);

		VkPipelineVertexInputStateCreateInfo VertexInputStateCI = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		VertexInputStateCI.vertexBindingDescriptionCount        = ARRAY_SIZE(VertexInputBindingDescriptions);
		VertexInputStateCI.pVertexBindingDescriptions           = VertexInputBindingDescriptions;
		VertexInputStateCI.vertexAttributeDescriptionCount      = ARRAY_SIZE(VertexInputAttributeDescriptions);
		VertexInputStateCI.pVertexAttributeDescriptions         = VertexInputAttributeDescriptions;

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		InputAssemblyStateCI.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssemblyStateCI.primitiveRestartEnable                 = VK_FALSE;

		VkViewport Viewport = {};
		Viewport.x          = 0.0f;
		Viewport.y          = 0.0f;
		Viewport.width      = (f32)Swapchain.Extent.width;
		Viewport.height     = (f32)Swapchain.Extent.height;
		Viewport.minDepth   = 0.0f;
		Viewport.maxDepth   = 1.0f;

		VkRect2D Scissor = {};
		Scissor.offset   = {0, 0};
		Scissor.extent   = Swapchain.Extent;

		VkPipelineViewportStateCreateInfo ViewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		ViewportState.viewportCount                     = 1;
		ViewportState.pViewports                        = &Viewport;
		ViewportState.scissorCount                      = 1;
		ViewportState.pScissors                         = &Scissor;

		VkPipelineRasterizationStateCreateInfo RasterizationStateCI = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		RasterizationStateCI.depthClampEnable                       = VK_FALSE;
		RasterizationStateCI.rasterizerDiscardEnable                = VK_FALSE;
		RasterizationStateCI.polygonMode                            = VK_POLYGON_MODE_FILL;
		RasterizationStateCI.lineWidth                              = 1.0f;
		RasterizationStateCI.depthBiasEnable                        = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo MultisampleStateCI = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		MultisampleStateCI.sampleShadingEnable                  = VK_FALSE;
		MultisampleStateCI.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
		ColorBlendAttachmentState.colorWriteMask                      = COLOR_MASK_RGBA;
		ColorBlendAttachmentState.blendEnable                         = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo ColorBlendStateCI = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		ColorBlendStateCI.logicOpEnable                       = VK_FALSE;
		ColorBlendStateCI.attachmentCount                     = 1;
		ColorBlendStateCI.pAttachments                        = &ColorBlendAttachmentState;

		VkPipelineLayoutCreateInfo PipelineLayoutCI = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		VK_CHECK(vkCreatePipelineLayout(Device, &PipelineLayoutCI, nullptr, &PipelineLayout));

		RenderPass = CreateRenderPass(Device, Swapchain.Format);

		VkGraphicsPipelineCreateInfo GraphicsPipelineCI = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		GraphicsPipelineCI.stageCount                   = 2;
		GraphicsPipelineCI.pStages                      = ShaderStageCIs;
		GraphicsPipelineCI.pVertexInputState            = &VertexInputStateCI;
		GraphicsPipelineCI.pInputAssemblyState          = &InputAssemblyStateCI;
		GraphicsPipelineCI.pViewportState               = &ViewportState;
		GraphicsPipelineCI.pRasterizationState          = &RasterizationStateCI;
		GraphicsPipelineCI.pMultisampleState            = &MultisampleStateCI;
		GraphicsPipelineCI.pDepthStencilState           = &DepthStencilStateCI;
		GraphicsPipelineCI.pDepthStencilState           = nullptr;
		GraphicsPipelineCI.pColorBlendState             = &ColorBlendStateCI;
		GraphicsPipelineCI.pDynamicState                = nullptr;
		GraphicsPipelineCI.layout                       = PipelineLayout;
		GraphicsPipelineCI.renderPass                   = RenderPass;
		GraphicsPipelineCI.subpass                      = 0;
		GraphicsPipelineCI.basePipelineHandle           = VK_NULL_HANDLE;
		GraphicsPipelineCI.basePipelineIndex            = -1;

		VK_CHECK(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &GraphicsPipelineCI, nullptr, &Pipeline));

		vkDestroyShaderModule(Device, VertexShader, nullptr);
		vkDestroyShaderModule(Device, FragmentShader, nullptr);
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

	VkPipeline       Pipeline;
	VkRenderPass     RenderPass;
	VkPipelineLayout PipelineLayout;

	VkCommandPool   CommandPool;
	VkCommandBuffer CommandBuffer;

	GBuffer VertexBuffer;
	GBuffer IndexBuffer;
};

int main()
{
	GHelloTriangleApplication App;
	App.Run();

	return 0;
}
