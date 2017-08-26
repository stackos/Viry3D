#include "MemoryStream.h"

namespace Viry3D
{
	MemoryStream::MemoryStream(const ByteBuffer& buffer):
		m_buffer(buffer)
	{
		m_length = m_buffer.Size();
	}

	int MemoryStream::Read(void* buffer, int size)
	{
		int pos = m_position;
		int read = Stream::Read(buffer, size);

		if(read > 0 && buffer != NULL)
		{
			Memory::Copy(buffer, &m_buffer[pos], read);
		}

		return read;
	}

	int MemoryStream::Write(void* buffer, int size)
	{
		int pos = m_position;
		int write = Stream::Write(buffer, size);

		if(write > 0 && buffer != NULL)
		{
			Memory::Copy(&m_buffer[pos], buffer, write);
		}

		return write;
	}

	String MemoryStream::ReadString(int size)
	{
		ByteBuffer buffer(size);
		Read(buffer.Bytes(), size);
		return String(buffer);
	}
}