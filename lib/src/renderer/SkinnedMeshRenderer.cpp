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

#include "SkinnedMeshRenderer.h"
#include "GameObject.h"
#include "graphics/Material.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(SkinnedMeshRenderer);

	SkinnedMeshRenderer::SkinnedMeshRenderer()
	{
	}

	void SkinnedMeshRenderer::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<SkinnedMeshRenderer>(source);
		auto mats = src->GetSharedMaterials();
		for (auto& i : mats)
		{
			auto mat_instance = Material::Create(i->GetShader()->GetName());
			mat_instance->DeepCopy(i);
			i = mat_instance;
		}
		this->SetSharedMaterials(mats);
		this->SetSharedMesh(src->GetSharedMesh());
		this->SetBones(src->GetBones());
	}

	const VertexBuffer* SkinnedMeshRenderer::GetVertexBuffer() const
	{
		return GetSharedMesh()->GetVertexBuffer().get();
	}

	const IndexBuffer* SkinnedMeshRenderer::GetIndexBuffer() const
	{
		return GetSharedMesh()->GetIndexBuffer().get();
	}

	void SkinnedMeshRenderer::GetIndexRange(int material_index, int& start, int& count) const
	{
		GetSharedMesh()->GetIndexRange(material_index, start, count);
	}

	bool SkinnedMeshRenderer::IsValidPass(int material_index) const
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

	void SkinnedMeshRenderer::PreRenderByRenderer(int material_index)
	{
		auto& mesh = this->GetSharedMesh();
		const auto& bindposes = mesh->bind_poses;
		const auto& bones = this->GetBones();
		Vector<Vector4> bone_matrix;
		const void* buffer;
		int size;

		if (bones.Size() > 0)
		{
			bone_matrix.Resize(bones.Size() * 3);
			for (int i = 0; i < bones.Size(); i++)
			{
				auto m = bones[i].lock()->GetLocalToWorldMatrix() * bindposes[i];
				bone_matrix[i * 3 + 0] = m.GetRow(0);
				bone_matrix[i * 3 + 1] = m.GetRow(1);
				bone_matrix[i * 3 + 2] = m.GetRow(2);
			}
			buffer = &bone_matrix[0];
			size = sizeof(Vector4) * bones.Size() * 3;
		}
		else
		{
			buffer = &this->GetTransform()->GetLocalToWorldMatrix();
			size = sizeof(Matrix4x4);
		}
		
		auto shader = this->GetSharedMaterials()[material_index]->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}
		shader->UpdateRendererDescriptorSet(m_descriptor_set, m_descriptor_set_buffer, buffer, size, m_lightmap_index);

		mesh->UpdateBlendShapes();
	}
}
