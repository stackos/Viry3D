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

#include "Mesh.h"
#include "Display.h"
#include "BufferObject.h"

namespace Viry3D
{
    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<unsigned short>& indices):
        m_vertex_count(0),
        m_index_count(0)
    {
        m_vertex_buffer = Display::Instance()->CreateBuffer(&vertices[0], vertices.SizeInBytes(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_index_buffer = Display::Instance()->CreateBuffer(&indices[0], indices.SizeInBytes(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    
        m_vertex_count = vertices.Size();
        m_index_count = indices.Size();
    }
    
    Mesh::~Mesh()
    {
        VkDevice device = Display::Instance()->GetDevice();

        m_vertex_buffer->Destroy(device);
        m_vertex_buffer.reset();
        m_index_buffer->Destroy(device);
        m_index_buffer.reset();
    }
}
