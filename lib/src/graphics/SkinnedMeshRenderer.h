/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#include "MeshRenderer.h"
#include "container/Map.h"

namespace Viry3D
{
    class SkinnedMeshRenderer : public MeshRenderer
    {
    public:
        SkinnedMeshRenderer();
        virtual ~SkinnedMeshRenderer();
		virtual void SetMesh(const Ref<Mesh>& mesh);
        const Vector<String>& GetBonePaths() const { return m_bone_paths; }
        void SetBonePaths(const Vector<String>& bones) { m_bone_paths = bones; }
        Ref<Transform> GetBonesRoot() const { return m_bones_root.lock(); }
        void SetBonesRoot(const Ref<Transform>& node) { m_bones_root = node; }
        float GetBlendShapeWeight(const String& name);
        void SetBlendShapeWeight(const String& name, float weight);
        const filament::backend::UniformBufferHandle& GetBonesUniformBuffer() const { return m_bones_uniform_buffer; }
        virtual Vector<filament::backend::RenderPrimitiveHandle> GetPrimitives();
        
	protected:
		virtual void PrepareRender();

    private:
        void FindBones();

	private:
		struct BlendShapeWeight
		{
			int index;
			float weight;
		};

    private:
        Vector<String> m_bone_paths;
        WeakRef<Transform> m_bones_root;
        Vector<WeakRef<Transform>> m_bones;
		Map<String, BlendShapeWeight> m_blend_shape_weights;
		bool m_blend_shape_dirty;
        filament::backend::UniformBufferHandle m_bones_uniform_buffer;
		filament::backend::VertexBufferHandle m_vb;
		Vector<filament::backend::RenderPrimitiveHandle> m_primitives;
		int m_vb_vertex_count;
		Vector<Mesh::Submesh> m_submeshes;
    };
}
