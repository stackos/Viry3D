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

#include "SkinnedMeshRenderer.h"
#include "GameObject.h"
#include "Engine.h"
#include "Debug.h"

namespace Viry3D
{
    SkinnedMeshRenderer::SkinnedMeshRenderer():
		m_blend_shape_dirty(false),
		m_vb_vertex_count(0)
    {

    }

    SkinnedMeshRenderer::~SkinnedMeshRenderer()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        if (m_bones_uniform_buffer)
        {
            driver.destroyUniformBuffer(m_bones_uniform_buffer);
			m_bones_uniform_buffer.clear();
        }

		if (m_vb)
		{
			driver.destroyVertexBuffer(m_vb);
			m_vb.clear();
		}
		for (int i = 0; i < m_primitives.Size(); ++i)
		{
			driver.destroyRenderPrimitive(m_primitives[i]);
			m_primitives[i].clear();
		}
		m_primitives.Clear();
    }

	void SkinnedMeshRenderer::SetMesh(const Ref<Mesh>& mesh)
	{
		MeshRenderer::SetMesh(mesh);

		m_blend_shape_weights.Clear();

		auto& driver = Engine::Instance()->GetDriverApi();
		if (m_vb)
		{
			driver.destroyVertexBuffer(m_vb);
			m_vb.clear();
		}
		for (int i = 0; i < m_primitives.Size(); ++i)
		{
			driver.destroyRenderPrimitive(m_primitives[i]);
			m_primitives[i].clear();
		}
		m_primitives.Clear();
	}

	float SkinnedMeshRenderer::GetBlendShapeWeight(const String& name)
	{
		float weight = 0;

		if (m_blend_shape_weights.Size() == 0)
		{
			const auto& mesh = this->GetMesh();
			if (mesh)
			{
				const auto& blend_shapes = mesh->GetBlendShapes();
				for (int i = 0; i < blend_shapes.Size(); ++i)
				{
					m_blend_shape_weights.Add(blend_shapes[i].name, { i, 0.0f });
				}
			}
		}

		const BlendShapeWeight* ptr;
		if (m_blend_shape_weights.TryGet(name, &ptr))
		{
			weight = ptr->weight;
		}

		return weight;
	}

	void SkinnedMeshRenderer::SetBlendShapeWeight(const String& name, float weight)
	{
		if (m_blend_shape_weights.Size() == 0)
		{
			const auto& mesh = this->GetMesh();
			if (mesh)
			{
				const auto& blend_shapes = mesh->GetBlendShapes();
				for (int i = 0; i < blend_shapes.Size(); ++i)
				{
					m_blend_shape_weights.Add(blend_shapes[i].name, { i, 0.0f });
				}
			}
		}

		BlendShapeWeight* ptr;
		if (m_blend_shape_weights.TryGet(name, &ptr))
		{
			ptr->weight = weight;

			m_blend_shape_dirty = true;
		}
	}

    void SkinnedMeshRenderer::FindBones()
    {
        auto root = m_bones_root.lock();
        const auto& root_name = root->GetName();

        m_bones.Resize(m_bone_paths.Size());
        for (int i = 0; i < m_bones.Size(); ++i)
        {
            if (m_bone_paths[i].StartsWith(root_name))
            {
                m_bones[i] = root->Find(m_bone_paths[i].Substring(root_name.Size() + 1));
            }
            
            if (m_bones[i].expired())
            {
                Log("can not find bone: %s", m_bone_paths[i].CString());
            }
        }
    }

    void SkinnedMeshRenderer::PrepareRender()
    {
		MeshRenderer::PrepareRender();

        const auto& materials = this->GetMaterials();
        const auto& mesh = this->GetMesh();

		// update bones
        if (materials.Size() > 0 && mesh && m_bone_paths.Size() > 0)
        {
            const auto& bindposes = mesh->GetBindposes();
            int bone_count = bindposes.Size();

            assert(m_bone_paths.Size() == bone_count);
            assert(m_bone_paths.Size() <= SkinnedMeshRendererUniforms::BONES_VECTOR_MAX_COUNT / 3);

            if (m_bones.Empty())
            {
                this->FindBones();
            }

            Vector<Vector4> bone_vectors(bone_count * 3);

            for (int i = 0; i < bone_count; ++i)
            {
                Matrix4x4 mat = m_bones[i].lock()->GetLocalToWorldMatrix() * bindposes[i];

                bone_vectors[i * 3 + 0] = mat.GetRow(0);
                bone_vectors[i * 3 + 1] = mat.GetRow(1);
                bone_vectors[i * 3 + 2] = mat.GetRow(2);
            }

            auto& driver = Engine::Instance()->GetDriverApi();
            if (!m_bones_uniform_buffer)
            {
                m_bones_uniform_buffer = driver.createUniformBuffer(sizeof(SkinnedMeshRendererUniforms), filament::backend::BufferUsage::DYNAMIC);
            }

			void* buffer = driver.allocate(bone_vectors.SizeInBytes());
            Memory::Copy(buffer, bone_vectors.Bytes(), bone_vectors.SizeInBytes());
            driver.loadUniformBuffer(m_bones_uniform_buffer, filament::backend::BufferDescriptor(buffer, bone_vectors.SizeInBytes()));
        }

		// update blend shapes
		if (m_blend_shape_dirty && mesh)
		{
			m_blend_shape_dirty = false;

			const auto& vertices = mesh->GetVertices();
			const auto& submeshes = mesh->GetSubmeshes();
			const auto& blend_shapes = mesh->GetBlendShapes();

			auto& driver = Engine::Instance()->GetDriverApi();

			Mesh::Vertex* buffer = (Mesh::Vertex*) driver.allocate(vertices.SizeInBytes());
			Memory::Copy(buffer, vertices.Bytes(), vertices.SizeInBytes());

			for (const auto& i : m_blend_shape_weights)
			{
				if (i.second.weight > 0)
				{
					const auto& shape = blend_shapes[i.second.index];

					for (int j = 0; j < vertices.Size(); ++j)
					{
						for (int k = 0; k < shape.frames.Size(); ++k)
						{
							const auto& frame = shape.frames[k];

							buffer[j].vertex += frame.vertices[j] * frame.weight * i.second.weight;
							buffer[j].normal += frame.normals[j] * frame.weight * i.second.weight;
							buffer[j].tangent += frame.tangents[j] * frame.weight * i.second.weight;
						}
					}
				}
			}

			if (m_vb_vertex_count != vertices.Size())
			{
				if (m_vb)
				{
					driver.destroyVertexBuffer(m_vb);
					m_vb.clear();
				}
			}
			if (!m_vb)
			{
				m_vb = driver.createVertexBuffer(1, (uint8_t) Shader::AttributeLocation::Count, vertices.Size(), mesh->GetAttributes(), filament::backend::BufferUsage::DYNAMIC);
				m_vb_vertex_count = vertices.Size();
			}

			if (m_submeshes.Size() != submeshes.Size() ||
				Memory::Compare(&m_submeshes[0], &submeshes[0], submeshes.SizeInBytes()) != 0)
			{
				for (int i = 0; i < m_primitives.Size(); ++i)
				{
					driver.destroyRenderPrimitive(m_primitives[i]);
					m_primitives[i].clear();
				}
				m_primitives.Clear();
				m_submeshes.Clear();
			}
			if (m_primitives.Size() == 0)
			{
				m_primitives.Resize(submeshes.Size());
				for (int i = 0; i < m_primitives.Size(); ++i)
				{
					m_primitives[i] = driver.createRenderPrimitive();

					driver.setRenderPrimitiveBuffer(m_primitives[i], m_vb, mesh->GetIndexBuffer(), mesh->GetEnabledAttributes());
					driver.setRenderPrimitiveRange(m_primitives[i], filament::backend::PrimitiveType::TRIANGLES, submeshes[i].index_first, 0, vertices.Size() - 1, submeshes[i].index_count);
				}
				m_submeshes = submeshes;
			}

			driver.updateVertexBuffer(m_vb, 0, filament::backend::BufferDescriptor(buffer, vertices.SizeInBytes()), 0);
		}
    }

    Vector<filament::backend::RenderPrimitiveHandle> SkinnedMeshRenderer::GetPrimitives()
    {
        Vector<filament::backend::RenderPrimitiveHandle> primitives;
        
		if (m_primitives.Size() > 0)
		{
			primitives = m_primitives;
		}
		else
		{
			const auto& mesh = this->GetMesh();
			if (mesh)
			{
				primitives = mesh->GetPrimitives();
			}
		}
        
        return primitives;
    }
}
