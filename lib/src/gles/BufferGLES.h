#pragma once

#include "gles_include.h"
#include "graphics/BufferType.h"
#include "memory/ByteBuffer.h"
#include <functional>

namespace Viry3D
{
	class BufferGLES
	{
	public:
		virtual ~BufferGLES();
		GLuint GetBuffer() const { return m_buffer; }
		int GetSize() const { return m_size; }
		const Ref<ByteBuffer>& GetLocalBuffer();

		typedef std::function<void(void* param, const ByteBuffer& buffer)> FillFunc;
		void Fill(void* param, FillFunc fill);

	protected:
		BufferGLES();
		void CreateInternal(BufferType::Enum type, bool dynamic = false);

		int m_size;

	private:
		GLuint m_buffer;
		GLenum m_type;
		GLenum m_usage;
		Ref<ByteBuffer> m_local_buffer;
	};
}
