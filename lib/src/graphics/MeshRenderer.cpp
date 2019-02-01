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

#include "MeshRenderer.h"
#include "Mesh.h"
#include "BufferObject.h"

namespace Viry3D
{
    MeshRenderer::MeshRenderer()
    {

    }

    MeshRenderer::~MeshRenderer()
    {
#if VR_VULKAN
        if (m_draw_buffer)
        {
            m_draw_buffer->Destroy(Display::Instance()->GetDevice());
            m_draw_buffer.reset();
        }
#endif
    }

    Ref<BufferObject> MeshRenderer::GetVertexBuffer() const
    {
        Ref<BufferObject> buffer;

        if (m_mesh)
        {
            buffer = m_mesh->GetVertexBuffer();
        }

        return buffer;
    }

    Ref<BufferObject> MeshRenderer::GetIndexBuffer() const
    {
        Ref<BufferObject> buffer;

        if (m_mesh)
        {
            buffer = m_mesh->GetIndexBuffer();
        }

        return buffer;
    }

    void MeshRenderer::SetMesh(const Ref<Mesh>& mesh)
    {
        m_mesh = mesh;
        m_draw_buffer_dirty = true;
    }

    void MeshRenderer::UpdateDrawBuffer()
    {
#if VR_VULKAN
        const auto& materials = this->GetMaterials();
        if (materials.Size() == 0 || !m_mesh)
        {
            if (m_draw_buffer)
            {
                m_draw_buffer->Destroy(Display::Instance()->GetDevice());
                m_draw_buffer.reset();

                this->MarkInstanceCmdDirty();
            }
            return;
        }

        Vector<VkDrawIndexedIndirectCommand> draws(materials.Size());
        int submesh_count = m_mesh->GetSubmeshCount();

        for (int i = 0; i < materials.Size(); ++i)
        {
            auto& draw = draws[i];
            if (i < submesh_count)
            {
                draw.indexCount = m_mesh->GetSubmesh(i).index_count;
                draw.firstIndex = m_mesh->GetSubmesh(i).index_first;
            }
            else
            {
                draw.indexCount = 0;
                draw.firstIndex = 0;
            }
            draw.instanceCount = this->GetInstanceCount();
            draw.vertexOffset = 0;
            draw.firstInstance = 0;
        }

        if (!m_draw_buffer || m_draw_buffer->GetSize() < draws.SizeInBytes())
        {
            if (m_draw_buffer)
            {
                m_draw_buffer->Destroy(Display::Instance()->GetDevice());
                m_draw_buffer.reset();
            }

            m_draw_buffer = Display::Instance()->CreateBuffer(draws.Bytes(), draws.SizeInBytes(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, VK_FORMAT_UNDEFINED);
            
            this->MarkInstanceCmdDirty();
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_draw_buffer, 0, draws.Bytes(), draws.SizeInBytes());
        }
#elif VR_GLES
        const auto& materials = this->GetMaterials();
        if (materials.Size() == 0 || !m_mesh)
        {
            m_draw_buffers.Clear();
            return;
        }

        m_draw_buffers.Resize(materials.Size());
        int submesh_count = m_mesh->GetSubmeshCount();

        for (int i = 0; i < materials.Size(); ++i)
        {
            auto& draw = m_draw_buffers[i];
            if (i < submesh_count)
            {
                draw.index_count = m_mesh->GetSubmesh(i).index_count;
                draw.first_index = m_mesh->GetSubmesh(i).index_first;
            }
            else
            {
                draw.index_count = 0;
                draw.first_index = 0;
            }
        }
#endif
    }
}
