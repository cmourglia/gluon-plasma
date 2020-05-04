#include "gvulkan.h"
#include "gswapchain.h"
#include "gbuffer.h"
#include "gshader.h"

#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <loguru.hpp>
#include <tiny_gltf.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <algorithm>

struct GVertex
{
	f32 Position[3];
	f32 Normal[3];
	f32 Texcoord[2];
};

struct GMesh
{
	std::vector<GVertex> Vertices;
	std::vector<u32>     Indices;
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
		LoadMesh("Assets/Meshes/Duck/Duck.gltf");
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

			VkDescriptorBufferInfo BufferInfo = {};
			BufferInfo.buffer                 = VertexBuffer.Buffer;
			BufferInfo.offset                 = 0;
			BufferInfo.range                  = VertexBuffer.Size;

			VkWriteDescriptorSet Descriptors[1] = {};
			Descriptors[0].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Descriptors[0].dstBinding           = 0;
			Descriptors[0].descriptorCount      = 1;
			Descriptors[0].descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			Descriptors[0].pBufferInfo          = &BufferInfo;

			vkCmdPushDescriptorSetKHR(CommandBuffer,
			                          VK_PIPELINE_BIND_POINT_GRAPHICS,
			                          PipelineLayout,
			                          0,
			                          ARRAY_SIZE(Descriptors),
			                          Descriptors);

			vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
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

		const char* DeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};

		Device = CreateLogicalDevice(PhysicalDevice, QueueIndices, ARRAY_SIZE(DeviceExtensions), DeviceExtensions);
		assert(Device);

		vkGetDeviceQueue(Device, QueueIndices.GraphicsFamily.Value(), 0, &GraphicsQueue);
		vkGetDeviceQueue(Device, QueueIndices.PresentFamily.Value(), 0, &PresentQueue);
		vkGetDeviceQueue(Device, QueueIndices.ComputeFamily.Value(), 0, &ComputeQueue);

		Swapchain = CreateSwapchain(PhysicalDevice, Device, Surface, 1024, 768, QueueIndices);

		GShader VertexShader   = LoadShader(Device, "Assets/Shaders/Bin/triangle.vert.spv");
		GShader FragmentShader = LoadShader(Device, "Assets/Shaders/Bin/triangle.frag.spv");

		RenderPass     = CreateRenderPass(Device, Swapchain.Format);
		PipelineLayout = CreatePipelineLayout(Device, VertexShader, FragmentShader);

		Pipeline = CreateGraphicsPipeline(Device, RenderPass, PipelineLayout, VertexShader, FragmentShader);

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

		VertexBuffer = CreateBuffer(Device, MemoryProperties, Mesh.Vertices.size() * sizeof(GVertex), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		IndexBuffer  = CreateBuffer(Device, MemoryProperties, Mesh.Indices.size() * sizeof(u32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		memcpy(VertexBuffer.Data, Mesh.Vertices.data(), Mesh.Vertices.size() * sizeof(GVertex));
		memcpy(IndexBuffer.Data, Mesh.Indices.data(), Mesh.Indices.size() * sizeof(u32));
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

	void LoadMesh(const char* Filename)
	{
		tinygltf::Model    Model;
		tinygltf::TinyGLTF Loader;

		std::string Error;
		std::string Warning;

		bool Result = Loader.LoadASCIIFromFile(&Model, &Error, &Warning, Filename);

		if (!Warning.empty())
		{
			LOG_F(WARNING, "Loader: %s", Warning.c_str());
		}

		if (!Error.empty())
		{
			LOG_F(ERROR, "Loader: %s", Error.c_str());
		}

		assert(Result);

		for (const auto& GLTFMesh : Model.meshes)
		{
			for (const auto& Primitive : GLTFMesh.primitives)
			{
				{
					tinygltf::Accessor   IndicesAccessor = Model.accessors[Primitive.indices];
					tinygltf::BufferView BufferView      = Model.bufferViews[IndicesAccessor.bufferView];
					tinygltf::Buffer     Buffer          = Model.buffers[BufferView.buffer];

					const u8* DataPtr    = Buffer.data.data() + BufferView.byteOffset + IndicesAccessor.byteOffset;
					const i32 ByteStride = IndicesAccessor.ByteStride(BufferView);
					const u64 Count      = IndicesAccessor.count;

					switch (IndicesAccessor.componentType)
					{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						{
							const u16* Ptr = (u16*)DataPtr;
							for (u64 Index = 0; Index < Count; ++Index)
							{
								Mesh.Indices.push_back(*(Ptr++));
							}
						}
						break;

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
						{
							const u32* Ptr = (u32*)DataPtr;
							for (u64 Index = 0; Index < Count; ++Index)
							{
								Mesh.Indices.push_back(*(Ptr++));
							}
						}
						break;
					}
				}

				assert(Primitive.mode == TINYGLTF_MODE_TRIANGLES);
				for (const auto& Attribute : Primitive.attributes)
				{
					const auto& AttributeAccessor = Model.accessors[Attribute.second];
					const auto& BufferView        = Model.bufferViews[AttributeAccessor.bufferView];
					const auto& Buffer            = Model.buffers[BufferView.buffer];

					const u8* DataPtr    = Buffer.data.data() + BufferView.byteOffset + AttributeAccessor.byteOffset;
					const i32 ByteStride = AttributeAccessor.ByteStride(BufferView);
					const u64 Count      = AttributeAccessor.count;

					if (Mesh.Vertices.empty())
					{
						Mesh.Vertices.resize(Count);
					}

					if (Attribute.first == "POSITION")
					{
						assert(AttributeAccessor.type == TINYGLTF_TYPE_VEC3);
						assert(AttributeAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

						const f32* Ptr = (f32*)DataPtr;
						for (u64 Index = 0; Index < Count; ++Index)
						{
							Mesh.Vertices[Index].Position[0] = *(Ptr++);
							Mesh.Vertices[Index].Position[1] = *(Ptr++);
							Mesh.Vertices[Index].Position[2] = *(Ptr++);
						}
					}
					else if (Attribute.first == "NORMAL")
					{
						assert(AttributeAccessor.type == TINYGLTF_TYPE_VEC3);
						assert(AttributeAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

						const f32* Ptr = (f32*)DataPtr;
						for (u64 Index = 0; Index < Count; ++Index)
						{
							Mesh.Vertices[Index].Normal[0] = *(Ptr++);
							Mesh.Vertices[Index].Normal[1] = *(Ptr++);
							Mesh.Vertices[Index].Normal[2] = *(Ptr++);
						}
					}
					else if (Attribute.first == "TEXCOORD_0")
					{
						assert(AttributeAccessor.type == TINYGLTF_TYPE_VEC2);
						assert(AttributeAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

						const f32* Ptr = (f32*)DataPtr;
						for (u64 Index = 0; Index < Count; ++Index)
						{
							Mesh.Vertices[Index].Texcoord[0] = *(Ptr++);
							Mesh.Vertices[Index].Texcoord[1] = *(Ptr++);
						}
					}
					else
					{
						LOG_F(WARNING, "%s is not handled", Attribute.first.c_str());
					}
				}
			}
		}
	}

private:
	GLFWwindow* Window;

	GMesh Mesh;

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
