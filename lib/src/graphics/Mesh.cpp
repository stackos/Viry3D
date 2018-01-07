/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
#include "VertexAttribute.h"

namespace Viry3D
{
	Mesh::Mesh():
		m_dynamic(false),
		m_blend_shape_dirty(false)
	{
		SetName("Mesh");
	}

	Ref<Mesh> Mesh::Create(bool dynamic)
	{
		Ref<Mesh> mesh(new Mesh());

		mesh->m_dynamic = dynamic;

		return mesh;
	}

	void Mesh::SetDynamic(bool dynamic)
	{
		if(!this->IsDynamic() && dynamic)
		{
			if (m_vertex_buffer)
			{
				m_vertex_buffer.reset();
			}

			if (m_index_buffer)
			{
				m_index_buffer.reset();
			}

			this->m_dynamic = dynamic;
		}
	}

	int Mesh::GetBlendShapeCount() const
	{
		return blend_shapes.Size();
	}

	const String& Mesh::GetBlendShapeName(int index) const
	{
		return blend_shapes[index].name;
	}

	float Mesh::GetBlendShapeWeight(int index) const
	{
		return blend_shapes[index].weight;
	}

	void Mesh::SetBlendShapeWeight(int index, float weight)
	{
		if (!Mathf::FloatEqual(blend_shapes[index].weight, weight))
		{
			blend_shapes[index].weight = weight;
			m_blend_shape_dirty = true;
		}
	}

	void Mesh::UpdateBlendShapes()
	{
		if (blend_shapes.Size() > 0 && m_blend_shape_dirty)
		{
			m_blend_shape_dirty = false;

			int vertex_count = this->vertices.Size();

			if (blend_shapes_deltas.Empty())
			{
				blend_shapes_deltas.Resize(vertex_count);
			}

			bool first_frame = true;

			for (int i = 0; i < blend_shapes.Size(); i++)
			{
				auto& shape = blend_shapes[i];

				if (shape.weight > 0)
				{
					for (int j = 0; j < shape.frames.Size(); j++)
					{
						auto& frame = shape.frames[j];

						if (frame.weight > 0)
						{
							for (int k = 0; k < vertex_count; k++)
							{
								float weight = shape.weight / 100.0f * frame.weight / 100.0f;

								if (first_frame)
								{
									blend_shapes_deltas[k].vertex = frame.deltas[k].vertex * weight;
									blend_shapes_deltas[k].normal = frame.deltas[k].normal * weight;
									blend_shapes_deltas[k].tangent = frame.deltas[k].tangent * weight;
								}
								else
								{
									blend_shapes_deltas[k].vertex += frame.deltas[k].vertex * weight;
									blend_shapes_deltas[k].normal += frame.deltas[k].normal * weight;
									blend_shapes_deltas[k].tangent += frame.deltas[k].tangent * weight;
								}
							}

							first_frame = false;
						}
					}
				}
			}

			if (!first_frame) // need update vertex buffer
			{
				this->UpdateVertexBuffer();
			}
		}
	}

	void Mesh::Apply()
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

		bool has_blend_shape = mesh->blend_shapes_deltas.Size() > 0;

		int count = mesh->vertices.Size();
		for (int i = 0; i < count; i++)
		{
			if (has_blend_shape)
			{
				ms.Write<Vector3>(mesh->vertices[i] + mesh->blend_shapes_deltas[i].vertex);
			}
			else
			{
				ms.Write<Vector3>(mesh->vertices[i]);
			}

			if (mesh->colors.Empty())
			{
				ms.Write<Color>(Color(1, 1, 1, 1));
			}
			else
			{
				ms.Write<Color>(mesh->colors[i]);
			}

			if (mesh->uv.Empty())
			{
				ms.Write<Vector2>(Vector2(0, 0));
			}
			else
			{
				ms.Write<Vector2>(mesh->uv[i]);
			}

			if (mesh->uv2.Empty())
			{
				ms.Write<Vector2>(Vector2(0, 0));
			}
			else
			{
				ms.Write<Vector2>(mesh->uv2[i]);
			}

			if (mesh->normals.Empty())
			{
				ms.Write<Vector3>(Vector3(0, 0, 0));
			}
			else
			{
				if (has_blend_shape)
				{
					ms.Write<Vector3>(mesh->normals[i] + mesh->blend_shapes_deltas[i].normal);
				}
				else
				{
					ms.Write<Vector3>(mesh->normals[i]);
				}
			}

			if (mesh->tangents.Empty())
			{
				ms.Write<Vector4>(Vector4(0, 0, 0, 0));
			}
			else
			{
				if (has_blend_shape)
				{
					ms.Write<Vector4>(mesh->tangents[i] + mesh->blend_shapes_deltas[i].tangent);
				}
				else
				{
					ms.Write<Vector4>(mesh->tangents[i]);
				}
			}

			if (mesh->bone_weights.Empty())
			{
				ms.Write<Vector4>(Vector4(0, 0, 0, 0));
			}
			else
			{
				ms.Write<Vector4>(mesh->bone_weights[i]);
			}

			if (mesh->bone_indices.Empty())
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

	int Mesh::GetSubmeshCount() const
	{
		if (submeshes.Empty())
		{
			return 1;
		}
		else
		{
			return submeshes.Size();
		}
	}
}
