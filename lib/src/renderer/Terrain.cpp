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

namespace Viry3D
{
	DEFINE_COM_CLASS(Terrain);

	Terrain::Terrain():
		m_tile_map_size(513),
		m_tile_noise_size(4),
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

	void Terrain::GenerateTile(int x, int z)
	{
		
	}
}
