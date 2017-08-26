#include "UniformBuffer.h"

namespace Viry3D
{
	Ref<UniformBuffer> UniformBuffer::Create(int size)
	{
		Ref<UniformBuffer> buffer(new UniformBuffer());

		buffer->m_size = size;
		buffer->CreateInternal(BufferType::Uniform);

		return buffer;
	}
}