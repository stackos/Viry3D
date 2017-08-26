#pragma once

#include "Stream.h"
#include "memory/ByteBuffer.h"
#include "memory/Memory.h"
#include "string/String.h"
#include <assert.h>

namespace Viry3D
{
	class MemoryStream : public Stream
	{
	public:
		MemoryStream(const ByteBuffer& buffer);
		virtual int Read(void* buffer, int size);
		virtual int Write(void* buffer, int size);
		template<class T>
		T Read();
		template<class T>
		void Write(const T& t);
		String ReadString(int size);

	private:
		ByteBuffer m_buffer;
	};

	template<class T>
	T MemoryStream::Read()
	{
		T t;
		Read((void*) &t, sizeof(T));
		return t;
	}

	template<class T>
	void MemoryStream::Write(const T& t)
	{
		Write((void*) &t, sizeof(T));
	}
}