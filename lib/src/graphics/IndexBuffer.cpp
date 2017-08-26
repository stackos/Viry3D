#include "IndexBuffer.h"

namespace Viry3D
{
	Ref<IndexBuffer> IndexBuffer::Create(int size, bool dynamic)
	{
		Ref<IndexBuffer> buffer(new IndexBuffer());

		buffer->m_size = size;
		buffer->CreateInternal(BufferType::Index, dynamic);

		return buffer;
	}
}