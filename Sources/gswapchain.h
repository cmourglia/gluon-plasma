#pragma once

#include "gvkcommon.h"

struct GSwapchain
{
	VkSwapchainKHR Swapchain;

	VkFormat      Format;
	VkExtent2D    Extent;
	u32           ImageCount;
	VkImage       Images[8];
	VkImageView   ImageViews[8];
	VkFramebuffer Frameubffers[8];
};

struct GQueueFamilyIndices;

GSwapchain CreateSwapchain(VkPhysicalDevice           PhysicalDevice,
                           VkDevice                   Device,
                           VkSurfaceKHR               Surface,
                           u32                        Width,
                           u32                        Height,
                           const GQueueFamilyIndices& QueueIndices);

void CreateSwapchainFramebuffers(VkDevice Device, VkRenderPass RenderPass, GSwapchain* Swapchain);

void DestroySwapchain(VkDevice Device, GSwapchain* Swapchain);
