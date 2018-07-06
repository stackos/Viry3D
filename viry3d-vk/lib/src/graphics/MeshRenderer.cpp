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
    MeshRenderer::MeshRenderer()
    {

    }

    MeshRenderer::~MeshRenderer()
    {
        if (m_draw_buffer)
        {
            m_draw_buffer->Destroy(Display::Instance()->GetDevice());
            m_draw_buffer.reset();
        }
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

        VkDrawIndexedIndirectCommand draw;
        draw.indexCount = m_mesh->GetIndexCount();
        draw.instanceCount = 1;
        draw.firstIndex = 0;
        draw.vertexOffset = 0;
        draw.firstInstance = 0;

        if (!m_draw_buffer)
        {
            m_draw_buffer = Display::Instance()->CreateBuffer(&draw, sizeof(draw), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_draw_buffer, 0, &draw, sizeof(draw));
        }

        this->MarkInstanceCmdDirty();
    }
}
