#pragma once

#include "Renderer.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
	class MeshRenderer : public Renderer
	{
		DECLARE_COM_CLASS(MeshRenderer, Renderer);
	public:
		virtual const VertexBuffer* GetVertexBuffer();
		virtual const IndexBuffer* GetIndexBuffer();
		virtual void GetIndexRange(int material_index, int& start, int& count);
		virtual bool IsValidPass(int material_index);
		const Ref<Mesh>& GetSharedMesh() const { return m_mesh; }
		void SetSharedMesh(const Ref<Mesh>& mesh) { m_mesh = mesh; }

	private:
		MeshRenderer();

	private:
		Ref<Mesh> m_mesh;
	};
}