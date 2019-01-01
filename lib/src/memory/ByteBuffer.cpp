/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ByteBuffer.h"
#include "Memory.h"

namespace Viry3D
{
	ByteBuffer::ByteBuffer(int size):
		m_size(size),
		m_bytes(nullptr),
		m_weak_ref(false)
	{
		if (m_size > 0)
		{
			m_ref_count = RefMake<bool>(true);
			m_bytes = Memory::Alloc<byte>(m_size);
		}
		else
		{
			m_size = 0;
		}
	}

	ByteBuffer::ByteBuffer(const ByteBuffer& buffer)
	{
		m_size = buffer.m_size;
		m_bytes = buffer.m_bytes;
		m_ref_count = buffer.m_ref_count;
		m_weak_ref = buffer.m_weak_ref;
	}

	ByteBuffer::ByteBuffer(byte* bytes, int size):
		m_size(size),
		m_bytes(bytes),
		m_weak_ref(true)
	{
	}

	ByteBuffer& ByteBuffer::operator =(const ByteBuffer& buffer)
	{
		Free();

		m_size = buffer.m_size;
		m_bytes = buffer.m_bytes;
		m_ref_count = buffer.m_ref_count;
		m_weak_ref = buffer.m_weak_ref;

		return *this;
	}

	ByteBuffer::~ByteBuffer()
	{
		Free();
	}

	void ByteBuffer::Free()
	{
		if (!m_weak_ref)
		{
			if (m_ref_count && m_ref_count.use_count() == 1)
			{
				if (m_bytes != nullptr)
				{
					Memory::Free(m_bytes);
				}
			}
		}
	}

	byte* ByteBuffer::Bytes() const
	{
		return m_bytes;
	}

	int ByteBuffer::Size() const
	{
		return m_size;
	}

	byte& ByteBuffer::operator [](int index)
	{
		return m_bytes[index];
	}

	const byte& ByteBuffer::operator [](int index) const
	{
		return m_bytes[index];
	}
}
