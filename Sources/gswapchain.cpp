#include "gswapchain.h"

#include "utils.h"
#include "gvulkan.h"
#include "gresources.h"

#include <vector>

struct GSwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR        Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR>   PresentModes;
};

static GSwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface);
static VkSurfaceFormatKHR       ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats);
static VkPresentModeKHR         ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes);
static VkExtent2D               ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& Capabilities, u32 Width, u32 Height);

GSwapchain CreateSwapchain(VkPhysicalDevice           PhysicalDevice,
                           VkDevice                   Device,
                           VkSurfaceKHR               Surface,
                           const GQueueFamilyIndices& QueueIndices,
                           VkSwapchainKHR             OldSwapchain)
{
	GSwapchainSupportDetails Details = QuerySwapchainSupport(PhysicalDevice, Surface);

	VkSurfaceFormatKHR SurfaceFormat = ChooseSwapchainSurfaceFormat(Details.Formats);
	VkPresentModeKHR   PresentMode   = ChooseSwapchainPresentMode(Details.PresentModes);

	VkSurfaceCapabilitiesKHR SurfaceCaps;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCaps));

	const u32 Width  = SurfaceCaps.currentExtent.width;
	const u32 Height = SurfaceCaps.currentExtent.height;

	u32 ImageCount = Details.Capabilities.maxImageCount > 0
	                     ? Min(Details.Capabilities.minImageCount + 1, Details.Capabilities.maxImageCount)
	                     : Details.Capabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR CreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	CreateInfo.surface                  = Surface;
	CreateInfo.minImageCount            = ImageCount;
	CreateInfo.imageFormat              = SurfaceFormat.format;
	CreateInfo.imageColorSpace          = SurfaceFormat.colorSpace;
	CreateInfo.imageExtent              = {Width, Height};
	CreateInfo.imageArrayLayers         = 1;
	CreateInfo.imageUsage               = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	CreateInfo.preTransform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	CreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.presentMode              = PresentMode;
	CreateInfo.clipped                  = VK_TRUE;
	CreateInfo.oldSwapchain             = OldSwapchain;

	if (QueueIndices.GraphicsFamily != QueueIndices.PresentFamily)
	{
		const u32 Indices[]              = {QueueIndices.GraphicsFamily.Value(), QueueIndices.PresentFamily.Value()};
		CreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = 2;
		CreateInfo.pQueueFamilyIndices   = Indices;
	}
	else
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VkSwapchainKHR Swapchain;
	VK_CHECK(vkCreateSwapchainKHR(Device, &CreateInfo, nullptr, &Swapchain));

	ImageCount        = 0;
	VkImage Images[8] = {};
	VK_CHECK(vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, nullptr));
	VK_CHECK(vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, Images));

	// for (u32 Index = 0; Index < Swapchain.ImageCount; ++Index)
	// {
	// 	Swapchain.ImageViews[Index] = CreateImageView(Device, Swapchain.Images[Index], Swapchain.Format);
	// }

	GSwapchain Result     = {};
	Result.Swapchain      = Swapchain;
	Result.PhysicalDevice = PhysicalDevice;
	Result.Device         = Device;
	Result.Surface        = Surface;
	Result.Format         = SurfaceFormat.format;
	Result.Width          = Width;
	Result.Height         = Height;
	Result.QueueIndices   = &QueueIndices;
	Result.ImageCount     = ImageCount;

	memcpy(Result.Images, Images, ImageCount * sizeof(VkImage));

	return Result;
}

// void CreateSwapchainFramebuffers(VkDevice Device, VkRenderPass RenderPass, GSwapchain* Swapchain)
// {
// 	for (u32 Index = 0; Index < Swapchain->ImageCount; ++Index)
// 	{
// 		Swapchain->Frameubffers[Index] = CreateFramebuffer(Device,
// 		                                                   RenderPass,
// 		                                                   1,
// 		                                                   &Swapchain->ImageViews[Index],
// 		                                                   Swapchain->Extent.width,
// 		                                                   Swapchain->Extent.height);
// 	}
// }

bool ResizeSwapchainIfNecessary(GSwapchain* Swapchain)
{
	VkSurfaceCapabilitiesKHR SurfaceCaps;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Swapchain->PhysicalDevice, Swapchain->Surface, &SurfaceCaps));

	const u32 NewWidth  = SurfaceCaps.currentExtent.width;
	const u32 NewHeight = SurfaceCaps.currentExtent.height;

	if (NewWidth == Swapchain->Width && NewHeight == Swapchain->Height)
	{
		return false;
	}

	GSwapchain OldSwapchain = *Swapchain;

	*Swapchain = CreateSwapchain(Swapchain->PhysicalDevice,
	                             Swapchain->Device,
	                             Swapchain->Surface,
	                             *Swapchain->QueueIndices,
	                             OldSwapchain.Swapchain);

	VK_CHECK(vkDeviceWaitIdle(Swapchain->Device));
	DestroySwapchain(Swapchain->Device, &OldSwapchain);

	return true;
}

void DestroySwapchain(VkDevice Device, GSwapchain* Swapchain)
{
	vkDestroySwapchainKHR(Device, Swapchain->Swapchain, nullptr);
}

static GSwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface)
{
	GSwapchainSupportDetails Details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Details.Capabilities);

	u32 FormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);
	Details.Formats.resize(FormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Details.Formats.data());

	u32 PresentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, nullptr);
	Details.PresentModes.resize(PresentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, Details.PresentModes.data());

	return Details;
}

static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
{
	for (const auto& Format : Formats)
	{
		if (Format.colorSpace == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return Format;
		}
	}

	return Formats[0];
}

static VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
	for (const auto& PresentMode : PresentModes)
	{
		if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return PresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& Capabilities, u32 Width, u32 Height)
{
	if (Capabilities.currentExtent.width != UINT32_MAX)
	{
		return Capabilities.currentExtent;
	}
	else
	{
		VkExtent2D ActualExtent;

		ActualExtent.width  = Clamp(Width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
		ActualExtent.height = Clamp(Height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

		return ActualExtent;
	}
}
