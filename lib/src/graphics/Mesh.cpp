#include "Mesh.h"
#include "io/MemoryStream.h"
#include "XMLShader.h"

namespace Viry3D
{
	Mesh::Mesh()
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

		if(!m_vertex_buffer || m_vertex_buffer->GetSize() < buffer_size)
		{
			m_vertex_buffer = VertexBuffer::Create(buffer_size, dynamic);
		}
		m_vertex_buffer->Fill(this, Mesh::FillVertexBuffer);
	}

	void Mesh::UpdateIndexBuffer()
	{
		int buffer_size = this->IndexBufferSize();
		bool dynamic = this->IsDynamic();

		if(!m_index_buffer || m_index_buffer->GetSize() < buffer_size)
		{
			m_index_buffer = IndexBuffer::Create(buffer_size, dynamic);
		}
		m_index_buffer->Fill(this, Mesh::FillIndexBuffer);
	}

	int Mesh::VertexBufferSize() const
	{
		return VERTEX_STRIDE * vertices.Size();
	}

	int Mesh::IndexBufferSize() const
	{
		return triangles.Size() * sizeof(unsigned short);
	}

	void Mesh::FillVertexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto mesh = (Mesh*) param;
		auto ms = MemoryStream(buffer);

		int count = mesh->vertices.Size();
		for(int i = 0; i < count; i++)
		{
			ms.Write<Vector3>(mesh->vertices[i]);

			if(mesh->colors.Empty())
			{
				ms.Write<Color>(Color(1, 1, 1, 1));
			}
			else
			{
				ms.Write<Color>(mesh->colors[i]);
			}

			if(mesh->uv.Empty())
			{
				ms.Write<Vector2>(Vector2(0, 0));
			}
			else
			{
				ms.Write<Vector2>(mesh->uv[i]);
			}

			if(mesh->uv2.Empty())
			{
				ms.Write<Vector2>(Vector2(0, 0));
			}
			else
			{
				ms.Write<Vector2>(mesh->uv2[i]);
			}

			if(mesh->normals.Empty())
			{
				ms.Write<Vector3>(Vector3(0, 0, 0));
			}
			else
			{
				ms.Write<Vector3>(mesh->normals[i]);
			}

			if(mesh->tangents.Empty())
			{
				ms.Write<Vector4>(Vector4(0, 0, 0, 0));
			}
			else
			{
				ms.Write<Vector4>(mesh->tangents[i]);
			}

			if(mesh->bone_weights.Empty())
			{
				ms.Write<Vector4>(Vector4(0, 0, 0, 0));
			}
			else
			{
				ms.Write<Vector4>(mesh->bone_weights[i]);
			}

			if(mesh->bone_indices.Empty())
			{
				ms.Write<Vector4>(Vector4(0, 0, 0, 0));
			}
			else
			{
				ms.Write<Vector4>(mesh->bone_indices[i]);
			}
		}

		ms.Close();
	}

	void Mesh::FillIndexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto mesh = (Mesh*) param;
		Memory::Copy(buffer.Bytes(), (void*) &mesh->triangles[0], mesh->IndexBufferSize());
	}

	void Mesh::GetIndexRange(int submesh_index, int& start, int& count)
	{
		if(submeshes.Empty())
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
}