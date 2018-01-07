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

#pragma once

#include "Renderer.h"
#include "graphics/Mesh.h"

#define BONE_MAX 80

namespace Viry3D
{
	class SkinnedMeshRenderer: public Renderer
	{
		DECLARE_COM_CLASS(SkinnedMeshRenderer, Renderer);
	public:
		virtual const VertexBuffer* GetVertexBuffer() const;
		virtual const IndexBuffer* GetIndexBuffer() const;
		virtual void GetIndexRange(int material_index, int& start, int& count) const;
		virtual bool IsValidPass(int material_index) const;
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
