#pragma once

#include "defines.h"
#include <volk.h>
#include <assert.h>

#define VK_CHECK(x)                                                                                                                        \
	do                                                                                                                                     \
	{                                                                                                                                      \
		VkResult _Result = (x);                                                                                                            \
		if (_Result != VK_SUCCESS)                                                                                                         \
		{                                                                                                                                  \
			assert(false);                                                                                                                 \
		}                                                                                                                                  \
	} while (false)

static constexpr u32 COLOR_MASK_RGB  = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
static constexpr u32 COLOR_MASK_RGBA = COLOR_MASK_RGB | VK_COLOR_COMPONENT_A_BIT;
