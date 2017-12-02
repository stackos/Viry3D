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
		m_tile_noise_size(4),
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

	}

	bool Terrain::IsValidPass(int material_index) const
	{
		return false;
	}

	void Terrain::Apply()
	{
		if (m_heightmap_size <= 0)
		{
			return;
		}

		Vertex* vertices = Memory::Alloc<Vertex>(sizeof(Vertex) * m_heightmap_size * m_heightmap_size);
		int* indices = Memory::Alloc<int>(sizeof(int) * (m_heightmap_size - 1) * (m_heightmap_size - 1) * 2 * 3);

		int k = 0;
		for (int i = 0; i < m_heightmap_size; i++)
		{
			for (int j = 0; j < m_heightmap_size; j++)
			{
				float height = m_heightmap_data[i * m_heightmap_size + j];
				Vertex& v = vertices[i * m_heightmap_size + j];
				float x = j / (m_heightmap_size - 1) * m_terrain_size.x;
				float y = height * m_terrain_size.y;
				float z = (m_heightmap_size - 1 - i) / (m_heightmap_size - 1) * m_terrain_size.z;

				v.vertex = Vector3(x, y, z);
				v.uv = Vector2(x, z);
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

		Memory::Free(vertices);
		Memory::Free(indices);
	}

	void Terrain::CalculateNormals(Vertex* vertices)
	{
		// calculate normals and tangents

	}

	void Terrain::GenerateTile(int x, int y)
	{
		m_tile = RefMake<TerrainTile>();
		m_tile->x = x;
		m_tile->y = y;
		m_tile->noise_pos.x = m_noise_center.x + x * m_tile_noise_size;
		m_tile->noise_pos.y = m_noise_center.y + y * m_tile_noise_size;
		m_tile->world_pos.x = x * m_terrain_size.x;
		m_tile->world_pos.y = 0;
		m_tile->world_pos.z = y * m_terrain_size.z;
		m_tile->height_map = ByteBuffer(m_heightmap_size * m_heightmap_size * 2);

		GenerateTileHeightMap();

		auto colors = ByteBuffer(m_heightmap_size * m_heightmap_size);
		for (int i = 0; i < m_heightmap_size; i++)
		{
			for (int j = 0; j < m_heightmap_size; j++)
			{
				int us = *((unsigned short*) &m_tile->height_map[i * m_heightmap_size * 2 + j * 2]);
				colors[i * m_heightmap_size + j] = us >> 8;
			}
		}
		m_tile->debug_image = Texture2D::Create(m_heightmap_size, m_heightmap_size, TextureFormat::R8, TextureWrapMode::Clamp, FilterMode::Point, false, colors);
	}

	void Terrain::GenerateTileHeightMap()
	{
		ByteBuffer& buffer = m_tile->height_map;

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
				unsigned short us = (unsigned short) (Mathf::Clamp01((row[j] + 1.0f) * 0.5f) * 65535);
				unsigned short* p = (unsigned short*) &buffer[i * m_heightmap_size * 2 + j * 2];
				*p = us;
			}
		}
	}
}
