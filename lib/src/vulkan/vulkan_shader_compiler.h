#pragma once

#include "vulkan_include.h"
#include "container/Vector.h"
#include "string/String.h"

namespace Viry3D
{
	void init_compiler();
	bool glsl_to_spv(const VkShaderStageFlagBits shader_type, const char* src, Vector<unsigned int>& spirv, String& error);
	void deinit_compiler();
}