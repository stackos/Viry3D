#pragma once

#include "Renderer.h"
#include "graphics/Mesh.h"

#define BONE_MAX 80

namespace Viry3D
{
	class SkinnedMeshRenderer : public Renderer
	{
		DECLARE_COM_CLASS(SkinnedMeshRenderer, Renderer);
	public:
		virtual const VertexBuffer* GetVertexBuffer();
		virtual const IndexBuffer* GetIndexBuffer();
		virtual void GetIndexRange(int material_index, int& start, int& count);
		virtual bool IsValidPass(int material_index);
		const Ref<Mesh>& GetSharedMesh() const { return m_mesh; }
		void SetSharedMesh(const Ref<Mesh>& mesh) { m_mesh = mesh; }
		const Vector<WeakRef<Transform>>& GetBones() const { return m_bones; }
		Vector<WeakRef<Transform>>& GetBones() { return m_bones; }
		void SetBones(const Vector<WeakRef<Transform>>& bones) { m_bones = bones; }

	protected:
		virtual void PreRenderByRenderer(int material_index);

	private:
		SkinnedMeshRenderer();

	private:
		Ref<Mesh> m_mesh;
		Vector<WeakRef<Transform>> m_bones;
	};
}