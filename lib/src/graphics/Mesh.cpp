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
#include "Debug.h"
#include "io/File.h"
#include "io/MemoryStream.h"

namespace Viry3D
{
    Ref<Mesh> Mesh::LoadFromFile(const String& path)
    {
        Ref<Mesh> mesh;

        if (File::Exist(path))
        {
            MemoryStream ms(File::ReadAllBytes(path));

            int name_size = ms.Read<int>();
            String mesh_name = ms.ReadString(name_size);

            Vector<Vertex>* vertices = new Vector<Vertex>();
            Vector<unsigned short>* indices = new Vector<unsigned short>();
            Vector<Submesh>* submeshes = new Vector<Submesh>();
            Vector<Matrix4x4>* bindposes = new Vector<Matrix4x4>();

            int vertex_count = ms.Read<int>();
            vertices->Resize(vertex_count);

            for (int i = 0; i < vertex_count; ++i)
            {
                (*vertices)[i].vertex = ms.Read<Vector3>();
            }

            int color_count = ms.Read<int>();
            for (int i = 0; i < color_count; ++i)
            {
                float r = ms.Read<byte>() / 255.0f;
                float g = ms.Read<byte>() / 255.0f;
                float b = ms.Read<byte>() / 255.0f;
                float a = ms.Read<byte>() / 255.0f;
                (*vertices)[i].color = Color(r, g, b, a);
            }

            int uv_count = ms.Read<int>();
            for (int i = 0; i < uv_count; ++i)
            {
                (*vertices)[i].uv = ms.Read<Vector2>();
            }

            int uv2_count = ms.Read<int>();
            for (int i = 0; i < uv2_count; ++i)
            {
                (*vertices)[i].uv2 = ms.Read<Vector2>();
            }

            int normal_count = ms.Read<int>();
            for (int i = 0; i < normal_count; ++i)
            {
                (*vertices)[i].normal = ms.Read<Vector3>();
            }

            int tangent_count = ms.Read<int>();
            for (int i = 0; i < tangent_count; ++i)
            {
                (*vertices)[i].tangent = ms.Read<Vector4>();
            }

            int bone_weight_count = ms.Read<int>();
            for (int i = 0; i < bone_weight_count; ++i)
            {
                (*vertices)[i].bone_weight = ms.Read<Vector4>();
                float index0 = (float) ms.Read<byte>();
                float index1 = (float) ms.Read<byte>();
                float index2 = (float) ms.Read<byte>();
                float index3 = (float) ms.Read<byte>();
                (*vertices)[i].bone_indices = Vector4(index0, index1, index2, index3);
            }

            int index_count = ms.Read<int>();
            indices->Resize(index_count);
            ms.Read(&(*indices)[0], indices->SizeInBytes());

            int submesh_count = ms.Read<int>();
            submeshes->Resize(submesh_count);
            ms.Read(&(*submeshes)[0], submeshes->SizeInBytes());

            int bindpose_count = ms.Read<int>();
            if (bindpose_count > 0)
            {
                bindposes->Resize(bindpose_count);
                ms.Read(&(*bindposes)[0], bindposes->SizeInBytes());
            }
            
            mesh = RefMake<Mesh>(*vertices, *indices, *submeshes);
            mesh->SetName(mesh_name);
            mesh->SetBindposes(*bindposes);

            delete vertices;
            delete indices;
            delete submeshes;
            delete bindposes;
        }
        else
        {
            Log("mesh file not exist: %s", path.CString());
        }

        return mesh;
    }

    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<unsigned short>& indices, const Vector<Submesh>& submeshes, bool dynamic):
        m_vertex_count(0),
        m_index_count(0),
        m_buffer_vertex_count(0),
        m_buffer_index_count(0)
    {
#if VR_VULKAN
        m_vertex_buffer = Display::Instance()->CreateBuffer(&vertices[0], vertices.SizeInBytes(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_FORMAT_UNDEFINED);
        m_index_buffer = Display::Instance()->CreateBuffer(&indices[0], indices.SizeInBytes(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_FORMAT_UNDEFINED);
#elif VR_GLES
        m_vertex_buffer = Display::Instance()->CreateBuffer(&vertices[0], vertices.SizeInBytes(), GL_ARRAY_BUFFER, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        m_index_buffer = Display::Instance()->CreateBuffer(&indices[0], indices.SizeInBytes(), GL_ELEMENT_ARRAY_BUFFER, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

        m_vertex_count = vertices.Size();
        m_index_count = indices.Size();
        m_buffer_vertex_count = m_vertex_count;
        m_buffer_index_count = m_index_count;
        m_submeshes = submeshes;
        if (m_submeshes.Empty())
        {
            m_submeshes.Add(Submesh({ 0, indices.Size() }));
        }
    }
    
    Mesh::~Mesh()
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();
        m_vertex_buffer->Destroy(device);
        m_index_buffer->Destroy(device);
#endif
        
        m_vertex_buffer.reset();
        m_index_buffer.reset();
    }

    void Mesh::Update(const Vector<Vertex>& vertices, const Vector<unsigned short>& indices, const Vector<Submesh>& submeshes)
    {
        assert(vertices.Size() <= m_buffer_vertex_count);
        assert(indices.Size() <= m_buffer_index_count);

        Display::Instance()->UpdateBuffer(m_vertex_buffer, 0, &vertices[0], vertices.SizeInBytes());
        Display::Instance()->UpdateBuffer(m_index_buffer, 0, &indices[0], indices.SizeInBytes());

        m_vertex_count = vertices.Size();
        m_index_count = indices.Size();
        m_submeshes = submeshes;
        if (m_submeshes.Empty())
        {
            m_submeshes.Add(Submesh({ 0, indices.Size() }));
        }
    }
}
