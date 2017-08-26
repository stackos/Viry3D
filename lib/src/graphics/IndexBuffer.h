#pragma once

#if VR_VULKAN
#include "vulkan/BufferVulkan.h"
#elif VR_GLES
#include "gles/BufferGLES.h"
#endif

#include "memory/Ref.h"

namespace Viry3D
{
	struct IndexType
	{
		enum Enum
		{
			UnsignedShort,
			UnsignedInt
		};
	};

#if VR_VULKAN
	class IndexBuffer : public BufferVulkan
	{
#elif VR_GLES
	class IndexBuffer : public BufferGLES
	{
#endif
	public:
		static Ref<IndexBuffer> Create(int size, bool dynamic = false);

	private:
		IndexBuffer() { }
	};
}