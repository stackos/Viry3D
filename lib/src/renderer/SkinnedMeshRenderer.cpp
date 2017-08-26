#include "SkinnedMeshRenderer.h"
#include "GameObject.h"
#include "graphics/Material.h"

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
		for(auto& i : mats)
		{
			auto mat_instance = Material::Create(i->GetShader()->GetName());
			mat_instance->DeepCopy(i);
			i = mat_instance;
		}
		this->SetSharedMaterials(mats);
		this->SetSharedMesh(src->GetSharedMesh());
		this->SetBones(src->GetBones());
	}

	const VertexBuffer* SkinnedMeshRenderer::GetVertexBuffer()
	{
		return GetSharedMesh()->GetVertexBuffer().get();
	}

	const IndexBuffer* SkinnedMeshRenderer::GetIndexBuffer()
	{
		return GetSharedMesh()->GetIndexBuffer().get();
	}

	void SkinnedMeshRenderer::GetIndexRange(int material_index, int& start, int& count)
	{
		GetSharedMesh()->GetIndexRange(material_index, start, count);
	}

	bool SkinnedMeshRenderer::IsValidPass(int material_index)
	{
		if(m_mesh)
		{
			int submesh_count = m_mesh->submeshes.Size();

			if(material_index == 0)
			{
				return true;
			}
			else if(material_index > 0 && submesh_count > 0 && material_index < submesh_count)
			{
				return true;
			}
		}

		return false;
	}

	void SkinnedMeshRenderer::PreRenderByRenderer(int material_index)
	{
		Renderer::PreRenderByRenderer(material_index);

		auto& mat = this->GetSharedMaterials()[material_index];
		const auto& bindposes = this->GetSharedMesh()->bind_poses;
		const auto& bones = this->GetBones();
		Vector<Vector4> bone_matrix(bones.Size() * 3);
		for(int i = 0; i < bones.Size(); i++)
		{
			auto m = bones[i].lock()->GetLocalToWorldMatrix() * bindposes[i];
			bone_matrix[i * 3 + 0] = m.GetRow(0);
			bone_matrix[i * 3 + 1] = m.GetRow(1);
			bone_matrix[i * 3 + 2] = m.GetRow(2);
		}
		mat->SetVectorArray("_Bones", bone_matrix);
	}
}