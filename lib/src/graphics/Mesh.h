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

#include "Object.h"
#include "VertexAttribute.h"
#include "container/Vector.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
    class BufferObject;

    class Mesh : public Object
    {
    public:
        struct Submesh
        {
            int index_first;
            int index_count;
        };

    public:
        static Ref<Mesh> LoadFromFile(const String& path);
        Mesh(const Vector<Vertex>& vertices, const Vector<unsigned short>& indices, const Vector<Submesh>& submeshes = Vector<Submesh>(), bool dynamic = false);
        virtual ~Mesh();
        void Update(const Vector<Vertex>& vertices, const Vector<unsigned short>& indices, const Vector<Submesh>& submeshes = Vector<Submesh>());
        const Ref<BufferObject>& GetVertexBuffer() const { return m_vertex_buffer; }
        const Ref<BufferObject>& GetIndexBuffer() const { return m_index_buffer; }
        int GetVertexCount() const { return m_vertex_count; }
        int GetIndexCount() const { return m_index_count; }
        const Submesh& GetSubmesh(int submesh) const { return m_submeshes[submesh]; }
        void SetBindposes(const Vector<Matrix4x4>& bindposes) { m_bindposes = bindposes; }
        const Vector<Matrix4x4>& GetBindposes() const { return m_bindposes; }

    private:
        Ref<BufferObject> m_vertex_buffer;
        Ref<BufferObject> m_index_buffer;
        int m_vertex_count;
        int m_index_count;
        int m_buffer_vertex_count;
        int m_buffer_index_count;
        Vector<Submesh> m_submeshes;
        Vector<Matrix4x4> m_bindposes;
    };
}
