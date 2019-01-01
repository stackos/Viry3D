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

		if (read > 0 && buffer != nullptr)
		{
			Memory::Copy(buffer, &m_buffer[pos], read);
		}

		return read;
	}

	int MemoryStream::Write(void* buffer, int size)
	{
		int pos = m_position;
		int write = Stream::Write(buffer, size);

		if (write > 0 && buffer != nullptr)
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
