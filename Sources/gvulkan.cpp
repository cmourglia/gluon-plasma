#include "gvulkan.h"

#include <loguru.hpp>

#include <stdio.h>

#include <vector>
#include <unordered_set>

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      Severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT             Type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
                                                    void*                                       UserData)
{
	if (Severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LOG_F(ERROR, "%s: %s", CallbackData->pMessageIdName, CallbackData->pMessage);
	}
	else if (Severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LOG_F(WARNING, "%s: %s", CallbackData->pMessageIdName, CallbackData->pMessage);
	}

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks*              pAllocator,
                                      VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
	auto Func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (Func != nullptr)
	{
		return Func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT Messenger, VkAllocationCallbacks* pAllocator)
{
	auto Func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
	if (Func != nullptr)
	{
		Func(Instance, Messenger, pAllocator);
	}
}

static GQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);

VkInstance CreateInstance(u32 ExtensionCount, const char** ExtensionNames, u32 LayerCount, const char** LayerNames)
{
	VkApplicationInfo AppInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	AppInfo.pApplicationName   = "Gluon";
	AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	AppInfo.pEngineName        = "No Engine";
	AppInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion         = VK_API_VERSION_1_0;

	VkInstanceCreateInfo CreateInfo    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	CreateInfo.pApplicationInfo        = &AppInfo;
	CreateInfo.enabledExtensionCount   = ExtensionCount;
	CreateInfo.ppEnabledExtensionNames = ExtensionNames;
	CreateInfo.enabledLayerCount       = LayerCount;
	CreateInfo.ppEnabledLayerNames     = LayerNames;

	VkInstance Instance;
	VK_CHECK(vkCreateInstance(&CreateInfo, nullptr, &Instance));

	return Instance;
}

VkPhysicalDevice PickPhysicalDevice(VkInstance Instance, VkSurfaceKHR Surface, GQueueFamilyIndices* pQueueIndices)
{
	u32 PhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
	vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

	for (const auto& PhysicalDevice : PhysicalDevices)
	{
		GQueueFamilyIndices QueueIndices = FindQueueFamilies(PhysicalDevice, Surface);
		if (QueueIndices.IsComplete())
		{
			*pQueueIndices = QueueIndices;
			return PhysicalDevice;
		}
	}

	return VK_NULL_HANDLE;
}

VkDevice CreateLogicalDevice(VkPhysicalDevice           PhysicalDevice,
                             const GQueueFamilyIndices& FamilyIndices,
                             u32                        ExtensionCount,
                             const char**               ExtensionNames)
{
	f32 QueuePriorities[] = {1.0f};

	std::unordered_set<u32> UniqueIndices;
	UniqueIndices.insert(FamilyIndices.GraphicsFamily.Value());
	UniqueIndices.insert(FamilyIndices.PresentFamily.Value());
	UniqueIndices.insert(FamilyIndices.ComputeFamily.Value());

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

	for (u32 Index : UniqueIndices)
	{
		VkDeviceQueueCreateInfo QueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
		QueueCreateInfo.queueFamilyIndex        = Index;
		QueueCreateInfo.queueCount              = 1;
		QueueCreateInfo.pQueuePriorities        = QueuePriorities;
		QueueCreateInfos.push_back(QueueCreateInfo);
	}

	VkPhysicalDeviceFeatures DeviceFeatures = {};

	VkDeviceCreateInfo CreateInfo      = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	CreateInfo.queueCreateInfoCount    = (u32)QueueCreateInfos.size();
	CreateInfo.pQueueCreateInfos       = QueueCreateInfos.data();
	CreateInfo.pEnabledFeatures        = &DeviceFeatures;
	CreateInfo.enabledExtensionCount   = ExtensionCount;
	CreateInfo.ppEnabledExtensionNames = ExtensionNames;

	VkDevice Device;
	VK_CHECK(vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &Device));

	return Device;
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat ImageFormat)
{
	VkImageViewCreateInfo CreateInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	CreateInfo.image                           = Image;
	CreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	CreateInfo.format                          = ImageFormat;
	CreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	CreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	CreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	CreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	CreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	CreateInfo.subresourceRange.baseMipLevel   = 0;
	CreateInfo.subresourceRange.levelCount     = 1;
	CreateInfo.subresourceRange.baseArrayLayer = 0;
	CreateInfo.subresourceRange.layerCount     = 1;

	VkImageView ImageView;
	VK_CHECK(vkCreateImageView(Device, &CreateInfo, nullptr, &ImageView));

	return ImageView;
}

VkShaderModule CreateShader(VkDevice Device, const char* Filename)
{
	FILE* File = fopen(Filename, "rb");

	if (nullptr == File)
	{
		return VK_NULL_HANDLE;
	}

	fseek(File, 0, SEEK_END);
	const u64 Size = ftell(File);
	rewind(File);

	std::vector<u8> Bytes(Size);
	fread(Bytes.data(), 1, Size, File);

	fclose(File);

	VkShaderModuleCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	CreateInfo.codeSize                 = Size;
	CreateInfo.pCode                    = (u32*)Bytes.data();

	VkShaderModule ShaderModule;
	VK_CHECK(vkCreateShaderModule(Device, &CreateInfo, nullptr, &ShaderModule));

	return ShaderModule;
}

VkRenderPass CreateRenderPass(VkDevice Device, VkFormat Format)
{
	VkAttachmentDescription ColorAttachment = {};
	ColorAttachment.format                  = Format;
	ColorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ColorAttachment.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentReference = {};
	ColorAttachmentReference.attachment            = 0;
	ColorAttachmentReference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments    = &ColorAttachmentReference;

	VkRenderPassCreateInfo RenderPassCI = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	RenderPassCI.attachmentCount        = 1;
	RenderPassCI.pAttachments           = &ColorAttachment;
	RenderPassCI.subpassCount           = 1;
	RenderPassCI.pSubpasses             = &Subpass;

	VkRenderPass RenderPass;
	VK_CHECK(vkCreateRenderPass(Device, &RenderPassCI, nullptr, &RenderPass));

	return RenderPass;
}

VkFramebuffer CreateFramebuffer(VkDevice     Device,
                                VkRenderPass RenderPass,
                                u32          AttachmentCount,
                                VkImageView* Attachments,
                                u32          Width,
                                u32          Height)
{
	VkFramebufferCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	CreateInfo.renderPass              = RenderPass;
	CreateInfo.attachmentCount         = AttachmentCount;
	CreateInfo.pAttachments            = Attachments;
	CreateInfo.width                   = Width;
	CreateInfo.height                  = Height;
	CreateInfo.layers                  = 1;

	VkFramebuffer Framebuffer;
	VK_CHECK(vkCreateFramebuffer(Device, &CreateInfo, nullptr, &Framebuffer));

	return Framebuffer;
}

VkSemaphore CreateSemaphore(VkDevice Device)
{
	VkSemaphoreCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

	VkSemaphore Semaphore;
	VK_CHECK(vkCreateSemaphore(Device, &CreateInfo, nullptr, &Semaphore));

	return Semaphore;
}

VkCommandPool CreateCommandPool(VkDevice Device, u32 FamilyIndex)
{
	VkCommandPoolCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	CreateInfo.queueFamilyIndex        = FamilyIndex;
	CreateInfo.flags                   = 0;

	VkCommandPool CommandPool;
	VK_CHECK(vkCreateCommandPool(Device, &CreateInfo, nullptr, &CommandPool));

	return CommandPool;
}

VkImageMemoryBarrier ImageBarrier(VkImage       Image,
                                  VkAccessFlags SrcAccessMask,
                                  VkAccessFlags DstAccessMask,
                                  VkImageLayout OldLayout,
                                  VkImageLayout NewLayout)
{
	VkImageMemoryBarrier Barrier        = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	Barrier.srcAccessMask               = SrcAccessMask;
	Barrier.dstAccessMask               = DstAccessMask;
	Barrier.oldLayout                   = OldLayout;
	Barrier.newLayout                   = NewLayout;
	Barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	Barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	Barrier.image                       = Image;
	Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	Barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return Barrier;
}

static GQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	GQueueFamilyIndices Indices;

	u32 QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

	for (u32 QueueIndex = 0; QueueIndex < (u32)QueueFamilies.size(); ++QueueIndex)
	{
		if (!Indices.GraphicsFamily.HasValue() && QueueFamilies[QueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Indices.GraphicsFamily = QueueIndex;
		}

		if (!Indices.ComputeFamily.HasValue() && QueueFamilies[QueueIndex].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			Indices.ComputeFamily = QueueIndex;
		}

		if (!Indices.PresentFamily.HasValue())
		{
			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, QueueIndex, Surface, &PresentSupport);

			if (PresentSupport)
			{
				Indices.PresentFamily = QueueIndex;
			}
		}
	}

	return Indices;
}

VkDebugUtilsMessengerEXT SetupDebugMessenger(VkInstance Instance)
{
#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT CreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
	CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	CreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	CreateInfo.pfnUserCallback = DebugCallback;
	CreateInfo.pUserData       = nullptr;

	VkDebugUtilsMessengerEXT Messenger;
	VK_CHECK(CreateDebugUtilsMessengerEXT(Instance, &CreateInfo, nullptr, &Messenger));

	return Messenger;
#endif
}

bool CheckValidationLayerSupport(u32 ValidationLayerCount, const char** ValidationLayerNames)
{
	u32 LayerCount;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&LayerCount, nullptr));

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data()));

	for (u32 Index = 0; Index < ValidationLayerCount; ++Index)
	{
		bool LayerFound = false;

		for (const auto& LayerProperties : AvailableLayers)
		{
			if (strcmp(LayerProperties.layerName, ValidationLayerNames[Index]) == 0)
			{
				LayerFound = true;
				break;
			}
		}

		if (!LayerFound)
		{
			return false;
		}
	}

	return true;
}
