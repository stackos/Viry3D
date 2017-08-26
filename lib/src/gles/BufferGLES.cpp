/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "BufferGLES.h"
#include "Debug.h"
#include "memory/Memory.h"

namespace Viry3D
{
	BufferGLES::BufferGLES():
		m_size(0),
		m_buffer(0),
		m_type(0),
		m_usage(0)
	{
	}

	BufferGLES::~BufferGLES()
	{
		glDeleteBuffers(1, &m_buffer);
	}

	const Ref<ByteBuffer>& BufferGLES::GetLocalBuffer()
	{
		if (!m_local_buffer)
		{
			m_local_buffer = RefMake<ByteBuffer>(m_size);
		}

		return m_local_buffer;
	}

	void BufferGLES::CreateInternal(BufferType::Enum type, bool dynamic)
	{
		LogGLError();

		switch (type)
		{
			case Viry3D::BufferType::Vertex:
				m_type = GL_ARRAY_BUFFER;
				if (dynamic)
				{
					m_usage = GL_DYNAMIC_DRAW;
				}
				else
				{
					m_usage = GL_STATIC_DRAW;
				}
				break;
			case Viry3D::BufferType::Index:
				m_type = GL_ELEMENT_ARRAY_BUFFER;
				if (dynamic)
				{
					m_usage = GL_DYNAMIC_DRAW;
				}
				else
				{
					m_usage = GL_STATIC_DRAW;
				}
				break;
			case Viry3D::BufferType::Uniform:
				m_type = GL_UNIFORM_BUFFER;
				m_usage = GL_DYNAMIC_DRAW;
				break;
			default:
				m_type = 0;
				m_usage = 0;
				break;
		}

		glGenBuffers(1, &m_buffer);

		if (m_usage == GL_DYNAMIC_DRAW)
		{
			glBindBuffer(m_type, m_buffer);
			glBufferData(m_type, m_size, NULL, m_usage);
			glBindBuffer(m_type, 0);
		}

		LogGLError();
	}

	void BufferGLES::Fill(void* param, FillFunc fill)
	{
		LogGLError();

		glBindBuffer(m_type, m_buffer);

		if (m_usage == GL_DYNAMIC_DRAW)
		{
			/*
			void* mapped = glMapBufferRange(m_type, 0, m_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			ByteBuffer buffer((byte*) mapped, m_size);
			fill(param, buffer);

			auto unmap_result = glUnmapBuffer(m_type);
			if(unmap_result == GL_FALSE)
			{
			Log("glUnmapBuffer failed type:", m_type);
			}
			*/

			auto& buffer = *this->GetLocalBuffer().get();
			fill(param, buffer);
			glBufferSubData(m_type, 0, m_size, buffer.Bytes());
		}
		else
		{
			ByteBuffer buffer(m_size);
			fill(param, buffer);
			glBufferData(m_type, m_size, buffer.Bytes(), m_usage);
		}

		glBindBuffer(m_type, 0);

		LogGLError();
	}
}
