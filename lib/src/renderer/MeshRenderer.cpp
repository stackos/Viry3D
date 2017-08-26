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

	const VertexBuffer* MeshRenderer::GetVertexBuffer()
	{
		return GetSharedMesh()->GetVertexBuffer().get();
	}

	const IndexBuffer* MeshRenderer::GetIndexBuffer()
	{
		return GetSharedMesh()->GetIndexBuffer().get();
	}

	void MeshRenderer::GetIndexRange(int material_index, int& start, int& count)
	{
		GetSharedMesh()->GetIndexRange(material_index, start, count);
	}

	bool MeshRenderer::IsValidPass(int material_index)
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
}