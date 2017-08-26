#include "Stream.h"

namespace Viry3D
{
	Stream::Stream():
		m_position(0),
		m_closed(false),
		m_length(0)
	{
	}

	void Stream::Close()
	{
		m_closed = true;
	}

	int Stream::Read(void* buffer, int size)
	{
		int read;

		if(m_position + size <= m_length)
		{
			read = size;
		}
		else
		{
			read = m_length - m_position;
		}

		m_position += read;

		return read;
	}

	int Stream::Write(void* buffer, int size)
	{
		int write;

		if(m_position + size <= m_length)
		{
			write = size;
		}
		else
		{
			write = m_length - m_position;
		}

		m_position += write;

		return write;
	}
}