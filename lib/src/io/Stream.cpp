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

		if (m_position + size <= m_length)
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

		if (m_position + size <= m_length)
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
