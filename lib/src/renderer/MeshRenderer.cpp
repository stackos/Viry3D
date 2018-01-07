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

#include "MeshRenderer.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(MeshRenderer);

	MeshRenderer::MeshRenderer()
	{
	}

	void MeshRenderer::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<MeshRenderer>(source);
		this->SetSharedMesh(src->GetSharedMesh());
	}

	const VertexBuffer* MeshRenderer::GetVertexBuffer() const
	{
		return GetSharedMesh()->GetVertexBuffer().get();
	}

	const IndexBuffer* MeshRenderer::GetIndexBuffer() const
	{
		return GetSharedMesh()->GetIndexBuffer().get();
	}

	void MeshRenderer::GetIndexRange(int material_index, int& start, int& count) const
	{
		GetSharedMesh()->GetIndexRange(material_index, start, count);
	}

	bool MeshRenderer::IsValidPass(int material_index) const
	{
		if (m_mesh)
		{
			int submesh_count = m_mesh->submeshes.Size();

			if (material_index == 0)
			{
				return true;
			}
			else if (material_index > 0 && submesh_count > 0 && material_index < submesh_count)
			{
				return true;
			}
		}

		return false;
	}
}
