#pragma once

#include "gvkcommon.h"

struct GQueueFamilyIndices;

struct GSwapchain
{
	VkSwapchainKHR Swapchain;

	VkPhysicalDevice PhysicalDevice;
	VkDevice         Device;
	VkSurfaceKHR     Surface;

	const GQueueFamilyIndices* QueueIndices;

	VkFormat Format;
	u32      Width, Height;
	u32      ImageCount;
	VkImage  Images[8];
};

GSwapchain CreateSwapchain(VkPhysicalDevice           PhysicalDevice,
                           VkDevice                   Device,
                           VkSurfaceKHR               Surface,
                           const GQueueFamilyIndices& QueueIndices,
                           VkSwapchainKHR             OldSwapchain = VK_NULL_HANDLE);

bool ResizeSwapchainIfNecessary(GSwapchain* Swapchain);

void DestroySwapchain(VkDevice Device, GSwapchain* Swapchain);
