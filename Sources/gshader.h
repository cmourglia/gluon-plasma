#pragma once

#include "gvulkan.h"

struct GShader
{
	VkShaderModule Shader;
};

GShader LoadShader(VkDevice Device, const char* Filename);
