#pragma once

#if VR_VULKAN
#include "vulkan/BufferVulkan.h"
#elif VR_GLES
#include "gles/BufferGLES.h"
#endif

#include "memory/Ref.h"

namespace Viry3D
{
#if VR_VULKAN
	class ImageBuffer : public BufferVulkan
	{
#elif VR_GLES
	class ImageBuffer : public BufferGLES
	{
#endif
	public:
		static Ref<ImageBuffer> Create(int size);

	private:
		ImageBuffer() { }
	};
}