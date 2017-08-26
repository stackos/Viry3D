#pragma once

#include "memory/Ref.h"

namespace Viry3D
{
	typedef unsigned char byte;

	class ByteBuffer
	{
	public:
		ByteBuffer(int size = 0);
		ByteBuffer(const ByteBuffer& buffer);
		ByteBuffer(byte* bytes, int size);
		~ByteBuffer();

		byte* Bytes() const;
		int Size() const;

		ByteBuffer& operator =(const ByteBuffer& buffer);
		byte& operator [](int index);
		const byte& operator [](int index) const;

	private:
		void Free();

		int m_size;
		byte* m_bytes;
		Ref<bool> m_ref_count;
		bool m_weak_ref;
	};
}