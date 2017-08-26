#pragma once

#if VR_VULKAN
#include "vulkan/DisplayVulkan.h"
#elif VR_GLES
#include "gles/DisplayGLES.h"
#endif

namespace Viry3D
{
#if VR_VULKAN
	class Display : public DisplayVulkan { };
#elif VR_GLES
	class Display : public DisplayGLES { };
#endif
}