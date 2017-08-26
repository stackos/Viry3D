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
	class VertexBuffer : public BufferVulkan
	{
#elif VR_GLES
	class VertexBuffer : public BufferGLES
	{
#endif
	public:
		static Ref<VertexBuffer> Create(int size, bool dynamic = false);

	private:
		VertexBuffer() { }
	};
}