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

namespace Viry3D
{
	DEFINE_COM_CLASS(Terrain);

	Terrain::Terrain():
		m_tile_map_size(513),
		m_tile_noise_size(4),
		m_tile_world_unit(513),
		m_noise_center(0, 0)
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
		return NULL;
	}

	const IndexBuffer* Terrain::GetIndexBuffer() const
	{
		return NULL;
	}

	void Terrain::GetIndexRange(int material_index, int& start, int& count) const
	{

	}

	bool Terrain::IsValidPass(int material_index) const
	{
		return false;
	}

	TerrainTile* Terrain::GenerateTile(int x, int y)
	{
		TerrainTile tile;
		tile.x = x;
		tile.y = y;
		tile.noise_pos.x = m_noise_center.x + x * m_tile_noise_size;
		tile.noise_pos.y = m_noise_center.y + y * m_tile_noise_size;
		tile.world_pos.x = x * m_tile_world_unit;
		tile.world_pos.y = 0;
		tile.world_pos.z = y * m_tile_world_unit;

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
		builder.SetDestSize(m_tile_map_size, m_tile_map_size);
		float noise_x_min = tile.noise_pos.x - m_tile_noise_size / 2;
		float noise_x_max = tile.noise_pos.x + m_tile_noise_size / 2;
		float noise_z_min = tile.noise_pos.y - m_tile_noise_size / 2;
		float noise_z_max = tile.noise_pos.y + m_tile_noise_size / 2;
		builder.SetBounds(noise_x_min, noise_x_max, noise_z_min, noise_z_max);
		builder.Build();

		auto colors = ByteBuffer(m_tile_map_size * m_tile_map_size);
		for (int i = 0; i < m_tile_map_size; i++)
		{
			float* row = map.GetSlabPtr(i);
			for (int j = 0; j < m_tile_map_size; j++)
			{
				byte a = (byte) Mathf::Min((int) ((row[j] + 1) * 0.5f * 255), 255);
				colors[i * m_tile_map_size + j] = a;
			}
		}
		tile.debug_image = Texture2D::Create(m_tile_map_size, m_tile_map_size, TextureFormat::Alpha8, TextureWrapMode::Clamp, FilterMode::Point, false, colors);

		m_tiles.AddLast(tile);

		return &m_tiles.End()->prev->value;
	}
}
