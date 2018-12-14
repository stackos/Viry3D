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
#include "Mesh.h"
#include "BufferObject.h"

namespace Viry3D
{
    MeshRenderer::MeshRenderer():
        m_submesh(-1)
    {
#if VR_GLES
        m_draw_buffer.first_index = 0;
        m_draw_buffer.index_count = 0;
#endif
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

    void MeshRenderer::SetMesh(const Ref<Mesh>& mesh, int submesh)
    {
        m_mesh = mesh;
        m_submesh = submesh;
        m_draw_buffer_dirty = true;
    }

    void MeshRenderer::UpdateDrawBuffer()
    {
#if VR_VULKAN
        VkDrawIndexedIndirectCommand draw;
        draw.indexCount = m_mesh->GetSubmesh(m_submesh).index_count;
        draw.instanceCount = this->GetInstanceCount();
        draw.firstIndex = m_mesh->GetSubmesh(m_submesh).index_first;
        draw.vertexOffset = 0;
        draw.firstInstance = 0;

        if (!m_draw_buffer)
        {
            m_draw_buffer = Display::Instance()->CreateBuffer(&draw, sizeof(draw), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_FORMAT_UNDEFINED);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_draw_buffer, 0, &draw, sizeof(draw));
        }

        this->MarkInstanceCmdDirty();
#elif VR_GLES
        m_draw_buffer.first_index = m_mesh->GetSubmesh(m_submesh).index_first;
        m_draw_buffer.index_count = m_mesh->GetSubmesh(m_submesh).index_count;
#endif
    }
}
