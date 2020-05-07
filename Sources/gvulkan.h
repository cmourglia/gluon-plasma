#pragma once

#include "gvkcommon.h"
#include "utils.h"

struct GQueueFamilyIndices
{
	Optional<u32> GraphicsFamily;
	Optional<u32> PresentFamily;
	Optional<u32> ComputeFamily;

	inline bool IsComplete()
	{
		return GraphicsFamily.HasValue() && PresentFamily.HasValue() && ComputeFamily.HasValue();
	}
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks*              pAllocator,
                                      VkDebugUtilsMessengerEXT*                 pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT Messenger, VkAllocationCallbacks* pAllocator);

VkInstance       CreateInstance(u32 ExtensionCount, const char** RequiredExtensionNames, u32 LayerCount, const char** RequiredLayerNames);
VkPhysicalDevice PickPhysicalDevice(VkInstance Instance, VkSurfaceKHR Surface, GQueueFamilyIndices* QueueIndices);
VkDevice         CreateLogicalDevice(VkPhysicalDevice           PhysicalDevice,
                                     const GQueueFamilyIndices& FamilyIndices,
                                     u32                        ExtensionCount,
                                     const char**               ExtensionNames);

VkDebugUtilsMessengerEXT SetupDebugMessenger(VkInstance Instance);

bool CheckValidationLayerSupport(u32 ValidationLayerCount, const char** ValidationLayerNames);

VkFramebuffer CreateFramebuffer(VkDevice Device, VkRenderPass, u32 AttachmentCount, VkImageView* Attachments, u32 Width, u32 Height);

VkSemaphore CreateSemaphore(VkDevice Device);

VkCommandPool CreateCommandPool(VkDevice Device, u32 FamilyIndex);
