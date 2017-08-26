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
	class UniformBuffer : public BufferVulkan
	{
#elif VR_GLES
	class UniformBuffer : public BufferGLES
	{
#endif
	public:
		static Ref<UniformBuffer> Create(int size);

	private:
		UniformBuffer() { }
	};
}