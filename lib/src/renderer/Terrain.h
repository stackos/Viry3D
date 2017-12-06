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

#pragma once

#include "renderer/Renderer.h"
#include "container/FastList.h"
#include "math/Vector2.h"
#include "graphics/Texture2D.h"

namespace Viry3D
{
	struct Vertex;

	struct TerrainTile
	{
		int x;
		int y;
		Vector2 noise_pos;
		Vector<float> height_map_data;
		Ref<Texture2D> debug_image;
	};

	struct TerrainSplatTexture
	{
		Ref<Texture2D> texture;
		Ref<Texture2D> normal;
		Vector2 tile_size;
		Vector2 tile_offset;
	};

	class Terrain : public Renderer
	{
		DECLARE_COM_CLASS(Terrain, Renderer);
	public:
		virtual ~Terrain();
		virtual const VertexBuffer* GetVertexBuffer() const;
		virtual const IndexBuffer* GetIndexBuffer() const;
		virtual void GetIndexRange(int material_index, int& start, int& count) const;
		virtual bool IsValidPass(int material_index) const;
		virtual IndexType GetIndexType() const { return IndexType::UnsignedInt; }

		void SetTileNoiseSize(float size) { m_tile_noise_size = size; }
		void SetNoiseCenter(const Vector2& noise_center) { m_noise_center = noise_center; }
		void GenerateTile(int x, int y);
		const Ref<TerrainTile>& GetTile() const { return m_tile; }
		const Vector3& GetTerrainSize() const { return m_terrain_size; }
		void SetTerrainSize(const Vector3& size) { m_terrain_size = size; }
		void SetHeightmapSize(int size) { m_heightmap_size = size; }
		void SetHeightmapData(const Vector<float>& data) { m_heightmap_data = data; }
		void SetAlphamapSize(int size) { m_alphamap_size = size; }
		const Vector<Ref<Texture2D>>& GetAlphamaps() const { return m_alphamaps; }
		void SetAlphamaps(const Vector<Ref<Texture2D>>& maps) { m_alphamaps = maps; }
		const Vector<TerrainSplatTexture>& GetSplatTextures() const { return m_splat_textures; }
		void SetSplatTextures(const Vector<TerrainSplatTexture>& textures) { m_splat_textures = textures; }
		void Apply();

	private:
		Terrain();
		void GenerateTileHeightMap();
		void CalculateNormals(Vertex* vertices);

	private:
		float m_tile_noise_size;
		Vector2 m_noise_center;
		Ref<TerrainTile> m_tile;
		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
		Vector3 m_terrain_size;
		int m_heightmap_size;
		Vector<float> m_heightmap_data;
		int m_alphamap_size;
		Vector<Ref<Texture2D>> m_alphamaps;
		Vector<TerrainSplatTexture> m_splat_textures;
	};
}
