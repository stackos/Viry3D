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

#include "Mesh.h"
#include "io/MemoryStream.h"
#include "XMLShader.h"

namespace Viry3D
{
	Mesh::Mesh():
		m_vertex_count(0),
		m_vertex_stride(0)
	{
		SetName("Mesh");

		m_dynamic = false;
	}

	Ref<Mesh> Mesh::Create(bool dynamic)
	{
		Ref<Mesh> mesh(new Mesh());

		mesh->m_dynamic = dynamic;

		return mesh;
	}

	void Mesh::Update()
	{
		this->UpdateVertexBuffer();
		this->UpdateIndexBuffer();
	}

	void Mesh::UpdateVertexBuffer()
	{
		int buffer_size = this->VertexBufferSize();
		bool dynamic = this->IsDynamic();

		if (!m_vertex_buffer || m_vertex_buffer->GetSize() < buffer_size)
		{
			m_vertex_buffer = VertexBuffer::Create(buffer_size, dynamic);
		}
		m_vertex_buffer->Fill(this, Mesh::FillVertexBuffer);
	}

	void Mesh::UpdateIndexBuffer()
	{
		int buffer_size = this->IndexBufferSize();
		bool dynamic = this->IsDynamic();

		if (!m_index_buffer || m_index_buffer->GetSize() < buffer_size)
		{
			m_index_buffer = IndexBuffer::Create(buffer_size, dynamic);
		}
		m_index_buffer->Fill(this, Mesh::FillIndexBuffer);
	}

	int Mesh::VertexBufferSize() const
	{
		return m_vertex_buffer_data.Size();
	}

	int Mesh::IndexBufferSize() const
	{
		return triangles.Size() * sizeof(unsigned short);
	}

	void Mesh::FillVertexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto mesh = (Mesh*) param;
		Memory::Copy(buffer.Bytes(), mesh->m_vertex_buffer_data.Bytes(), mesh->m_vertex_buffer_data.Size());
	}

	void Mesh::FillIndexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto mesh = (Mesh*) param;
		Memory::Copy(buffer.Bytes(), (void*) &mesh->triangles[0], mesh->IndexBufferSize());
	}

	void Mesh::GetIndexRange(int submesh_index, int& start, int& count)
	{
		if (submeshes.Empty())
		{
			start = 0;
			count = triangles.Size();
		}
		else
		{
			auto submesh = submeshes[submesh_index];

			start = submesh.start;
			count = submesh.count;
		}
	}

	bool Mesh::HasVertexAttribute(VertexAttributeType type) const
	{
		for (const auto& i : m_vertex_attribute_offsets)
		{
			if (i.type == type)
			{
				return true;
			}
		}

		return false;
	}

	void Mesh::ClearVertexAttributeOffsets()
	{
		m_vertex_attribute_offsets.Clear();
		m_vertex_stride = 0;
	}

	void Mesh::AddVertexAttributeOffset(const VertexAttributeOffset& offset)
	{
		m_vertex_attribute_offsets.Add(offset);
		m_vertex_stride += VERTEX_ATTR_SIZES[(int) offset.type];
	}
}
