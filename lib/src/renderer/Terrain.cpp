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

#include "Terrain.h"
#include "noise/noise.h"
#include "noise/noiseutils.h"
#include "graphics/VertexAttribute.h"
#include "memory/Memory.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Terrain);

	Terrain::Terrain():
		m_tile_noise_size(1),
		m_noise_center(0, 0),
		m_terrain_size(500, 500, 500),
		m_heightmap_size(0),
		m_alphamap_size(512)
	{
	}

	Terrain::~Terrain()
	{
	}

	void Terrain::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<Terrain>(source);
		this->m_tile_noise_size = src->m_tile_noise_size;
		this->m_noise_center = src->m_noise_center;
		this->m_tile = src->m_tile;
		this->m_vertex_buffer = src->m_vertex_buffer;
		this->m_index_buffer = src->m_index_buffer;;
		this->m_terrain_size = src->m_terrain_size;
		this->m_heightmap_size = src->m_heightmap_size;
		this->m_heightmap_data = src->m_heightmap_data;
		this->m_alphamap_size = src->m_alphamap_size;
		this->m_alphamaps = src->m_alphamaps;
		this->m_splat_textures = src->m_splat_textures;
	}

	const VertexBuffer* Terrain::GetVertexBuffer() const
	{
		if (m_vertex_buffer)
		{
			return m_vertex_buffer.get();
		}

		return NULL;
	}

	const IndexBuffer* Terrain::GetIndexBuffer() const
	{
		if (m_index_buffer)
		{
			return m_index_buffer.get();
		}

		return NULL;
	}

	void Terrain::GetIndexRange(int material_index, int& start, int& count) const
	{
		start = 0;
		count = (m_heightmap_size - 1) * (m_heightmap_size - 1) * 2 * 3;
	}

	bool Terrain::IsValidPass(int material_index) const
	{
		return m_heightmap_size > 0;
	}

	void Terrain::Apply()
	{
		if (m_heightmap_size <= 0)
		{
			return;
		}

		int vertex_buffer_size = sizeof(Vertex) * m_heightmap_size * m_heightmap_size;
		int index_buffer_size = sizeof(int) * (m_heightmap_size - 1) * (m_heightmap_size - 1) * 2 * 3;

		Vertex* vertices = Memory::Alloc<Vertex>(vertex_buffer_size);
		int* indices = Memory::Alloc<int>(index_buffer_size);

		int k = 0;
		for (int i = 0; i < m_heightmap_size; i++)
		{
			for (int j = 0; j < m_heightmap_size; j++)
			{
				float h = m_heightmap_data[i * m_heightmap_size + j];
				Vertex& v = vertices[i * m_heightmap_size + j];
				float x = j / (float) (m_heightmap_size - 1) * m_terrain_size.x;
				float y = h * m_terrain_size.y;
				float z = i / (float) (m_heightmap_size - 1) * m_terrain_size.z;

				v.vertex = Vector3(x, y, z);
				v.uv = Vector2(x, m_terrain_size.z - z);
				v.uv2 = Vector2(x / m_terrain_size.x, z / m_terrain_size.z);

				if (i < m_heightmap_size - 1 && j < m_heightmap_size - 1)
				{
					indices[k++] = i * m_heightmap_size + j;
					indices[k++] = (i + 1) * m_heightmap_size + (j + 1);
					indices[k++] = (i + 1) * m_heightmap_size + j;

					indices[k++] = i * m_heightmap_size + j;
					indices[k++] = i * m_heightmap_size + (j + 1);
					indices[k++] = (i + 1) * m_heightmap_size + (j + 1);
				}
			}
		}

		this->CalculateNormals(vertices);

		m_vertex_buffer = VertexBuffer::Create(vertex_buffer_size);
		m_vertex_buffer->Fill(NULL, [=](void* param, const ByteBuffer& buffer) {
			Memory::Copy(buffer.Bytes(), vertices, vertex_buffer_size);
		});

		m_index_buffer = IndexBuffer::Create(index_buffer_size);
		m_index_buffer->Fill(NULL, [=](void* param, const ByteBuffer& buffer) {
			Memory::Copy(buffer.Bytes(), indices, index_buffer_size);
		});

		Memory::Free(vertices);
		Memory::Free(indices);
	}

	void Terrain::CalculateNormals(Vertex* vertices)
	{
		Vector3* face_normals = Memory::Alloc<Vector3>(sizeof(Vector3) * (m_heightmap_size - 1) * (m_heightmap_size - 1) * 2);
		Vector3* face_tangents = Memory::Alloc<Vector3>(sizeof(Vector3) * (m_heightmap_size - 1) * (m_heightmap_size - 1) * 2);
		
		Vector3 face_shared_normals[6];
		Vector3 face_shared_tangents[6];
		for (int i = 0; i < m_heightmap_size; i++)
		{
			for (int j = 0; j < m_heightmap_size; j++)
			{
				if (i < m_heightmap_size - 1 && j < m_heightmap_size - 1)
				{
					Vector3 a = vertices[i * m_heightmap_size + j].vertex;
					Vector3 b = vertices[(i + 1) * m_heightmap_size + j].vertex;
					Vector3 c = vertices[(i + 1) * m_heightmap_size + (j + 1)].vertex;
					Vector3 d = vertices[i * m_heightmap_size + (j + 1)].vertex;

					Vector3 m = (b - a) * (c - a);
					Vector3 n = (c - a) * (d - a);

					face_normals[i * (m_heightmap_size - 1) * 2 + j * 2] = Vector3::Normalize(m);
					face_normals[i * (m_heightmap_size - 1) * 2 + j * 2 + 1] = Vector3::Normalize(n);

					// calculate tangent
					Vector2 uva = vertices[i * m_heightmap_size + j].uv;
					Vector2 uvb = vertices[(i + 1) * m_heightmap_size + j].uv;
					Vector2 uvc = vertices[(i + 1) * m_heightmap_size + (j + 1)].uv;
					Vector2 uvd = vertices[i * m_heightmap_size + (j + 1)].uv;

					Vector3 e1 = a - b;
					Vector3 e2 = c - b;
					Vector2 d1 = uva - uvb;
					Vector2 d2 = uvc - uvb;
					float f = 1.0f / (d1.x * d2.y - d2.x * d1.y);
					Vector3 t;
					t.x = f * (d2.x * e1.x - d1.x * e2.x);
					t.y = f * (d2.x * e1.y - d1.x * e2.y);
					t.z = f * (d2.x * e1.z - d1.x * e2.z);
					t = Vector3::Normalize(t);
					face_tangents[i * (m_heightmap_size - 1) * 2 + j * 2] = t;

					e1 = a - d;
					e2 = c - d;
					d1 = uva - uvd;
					d2 = uvc - uvd;
					f = 1.0f / (d1.x * d2.y - d2.x * d1.y);
					t.x = f * (d2.x * e1.x - d1.x * e2.x);
					t.y = f * (d2.x * e1.y - d1.x * e2.y);
					t.z = f * (d2.x * e1.z - d1.x * e2.z);
					t = Vector3::Normalize(t);
					face_tangents[i * (m_heightmap_size - 1) * 2 + j * 2 + 1] = t;
				}

				int shared_normal = 0;
				int shared_tangent = 0;
				if (i < m_heightmap_size - 1 && j < m_heightmap_size - 1)
				{
					if (j > 0 && i > 0)
					{
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];
						face_shared_normals[shared_normal++] = face_normals[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];
						face_shared_tangents[shared_tangent++] = face_tangents[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					}
					else if (j > 0)
					{
						face_shared_normals[shared_normal++] = face_normals[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];

						face_shared_tangents[shared_tangent++] = face_tangents[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					}
					else if (i > 0)
					{
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];

						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];
					}

					face_shared_normals[shared_normal++] = face_normals[i * (m_heightmap_size - 1) * 2 + j * 2];
					face_shared_normals[shared_normal++] = face_normals[i * (m_heightmap_size - 1) * 2 + j * 2 + 1];

					face_shared_tangents[shared_tangent++] = face_tangents[i * (m_heightmap_size - 1) * 2 + j * 2];
					face_shared_tangents[shared_tangent++] = face_tangents[i * (m_heightmap_size - 1) * 2 + j * 2 + 1];
				}
				else if (j == m_heightmap_size - 1) // right column
				{
					if (i > 0)
					{
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					}

					if (i < m_heightmap_size - 1)
					{
						face_shared_normals[shared_normal++] = face_normals[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];

						face_shared_tangents[shared_tangent++] = face_tangents[i * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					}
				}
				else if (i == m_heightmap_size - 1) // bottom row
				{
					if (j > 0)
					{
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];

						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2];
						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + (j - 1) * 2 + 1];
					}

					if (j < m_heightmap_size - 1)
					{
						face_shared_normals[shared_normal++] = face_normals[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];

						face_shared_tangents[shared_tangent++] = face_tangents[(i - 1) * (m_heightmap_size - 1) * 2 + j * 2];
					}
				}

				Vector3 normal(0, 0, 0);
				for (int k = 0; k < shared_normal; k++)
				{
					normal += face_shared_normals[k];
				}
				normal = Vector3::Normalize(normal * (1.0f / shared_normal));

				Vector3 tangent(0, 0, 0);
				for (int k = 0; k < shared_tangent; k++)
				{
					tangent += face_shared_tangents[k];
				}
				tangent = Vector3::Normalize(tangent * (1.0f / shared_tangent));

				vertices[i * m_heightmap_size + j].normal = normal;
				vertices[i * m_heightmap_size + j].tangent = Vector4(tangent.x, tangent.y, tangent.z, 1);
			}
		}
		
		Memory::Free(face_normals);
		Memory::Free(face_tangents);
	}

	void Terrain::GenerateTile(int x, int y)
	{
		m_tile = RefMake<TerrainTile>();
		m_tile->x = x;
		m_tile->y = y;
		m_tile->noise_pos.x = m_noise_center.x + x * m_tile_noise_size;
		m_tile->noise_pos.y = m_noise_center.y + y * m_tile_noise_size;
		m_tile->height_map_data.Resize(m_heightmap_size * m_heightmap_size);

		GenerateTileHeightMap();

		auto colors = ByteBuffer(m_heightmap_size * m_heightmap_size);
		for (int i = 0; i < m_heightmap_size; i++)
		{
			for (int j = 0; j < m_heightmap_size; j++)
			{
				float h = m_tile->height_map_data[i * m_heightmap_size + j ];
				colors[i * m_heightmap_size + j] = (byte) (Mathf::Clamp01(h) * 255);
			}
		}
		m_tile->debug_image = Texture2D::Create(m_heightmap_size, m_heightmap_size, TextureFormat::R8, TextureWrapMode::Clamp, FilterMode::Point, false, colors);
	}

	void Terrain::GenerateTileHeightMap()
	{
		module::RidgedMulti mountain;

		module::Billow base;
		base.SetFrequency(2.0);

		module::ScaleBias flat;
		flat.SetSourceModule(0, base);
		flat.SetScale(0.125);
		flat.SetBias(-0.75);

		module::Perlin type;
		type.SetFrequency(0.5);
		type.SetPersistence(0.25);

		module::Select selector;
		selector.SetSourceModule(0, flat);
		selector.SetSourceModule(1, mountain);
		selector.SetControlModule(type);
		selector.SetBounds(0.0, 1000.0);
		selector.SetEdgeFalloff(0.125);

		module::Turbulence final;
		final.SetSourceModule(0, selector);
		final.SetFrequency(4.0);
		final.SetPower(0.125);

		utils::NoiseMap map;
		utils::NoiseMapBuilderPlane builder;
		builder.SetSourceModule(final);
		builder.SetDestNoiseMap(map);
		builder.SetDestSize(m_heightmap_size, m_heightmap_size);
		float noise_x_min = m_tile->noise_pos.x - m_tile_noise_size / 2;
		float noise_x_max = m_tile->noise_pos.x + m_tile_noise_size / 2;
		float noise_z_min = m_tile->noise_pos.y - m_tile_noise_size / 2;
		float noise_z_max = m_tile->noise_pos.y + m_tile_noise_size / 2;
		builder.SetBounds(noise_x_min, noise_x_max, noise_z_min, noise_z_max);
		builder.Build();

		for (int i = 0; i < m_heightmap_size; i++)
		{
			float* row = map.GetSlabPtr(i);
			for (int j = 0; j < m_heightmap_size; j++)
			{
				m_tile->height_map_data[i * m_heightmap_size + j] = (row[j] + 1.0f) * 0.5f;
			}
		}
	}
}
