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

using UnityEngine;
using UnityEditor;

public partial class Exporter {
	static void WriteTerrain(Terrain terrain) {
		var data = terrain.terrainData;
		Vector3 terrain_size = data.size;
		int heightmap_size = data.heightmapResolution;
		float[,] heightmap_data = data.GetHeights(0, 0, heightmap_size, heightmap_size);
		int alphamap_size = data.alphamapResolution;
		Texture2D[] alphamaps = data.alphamapTextures;
		int alphamap_count = alphamaps.Length;
		var splats = data.splatPrototypes;
		int splat_count = splats.Length;

		m_writer.Write(terrain.lightmapIndex);
		WriteVector4(terrain.lightmapScaleOffset);

		WriteVector3(terrain_size);
		m_writer.Write(heightmap_size);
		for (int i = 0; i < heightmap_size; i++)
		{
			for (int j = 0; j < heightmap_size; j++)
			{
				float height = heightmap_data[i, j];
				m_writer.Write(height);
			}
		}

		m_writer.Write(alphamap_size);
		m_writer.Write(alphamap_count);
		for (int i = 0; i < alphamap_count; i++)
		{
			WriteTexture(alphamaps[i], i);
		}

		m_writer.Write(splat_count);
		for (int i = 0; i < splat_count; i++)
		{
			WriteTexture(splats[i].texture);
			WriteTexture(splats[i].normalMap);
			WriteVector2(splats[i].tileSize);
			WriteVector2(splats[i].tileOffset);
		}
	}
}