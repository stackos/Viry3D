#include "ImageBuffer.h"

namespace Viry3D
{
	Ref<ImageBuffer> ImageBuffer::Create(int size)
	{
		Ref<ImageBuffer> buffer(new ImageBuffer());

		buffer->m_size = size;
		buffer->CreateInternal(BufferType::Image);

		return buffer;
	}
}