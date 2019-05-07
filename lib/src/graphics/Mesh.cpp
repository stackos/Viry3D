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

#include "Mesh.h"
#include "Debug.h"
#include "Engine.h"
#include "Shader.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "memory/Memory.h"

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
            Vector<unsigned int>* indices = new Vector<unsigned int>();
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
            for (int i = 0; i < index_count; ++i)
            {
                (*indices)[i] = ms.Read<unsigned short>();
            }

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

    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<unsigned int>& indices, const Vector<Submesh>& submeshes, bool uint32_index, bool dynamic):
        m_vertex_count(vertices.Size()),
        m_index_count(indices.Size()),
        m_buffer_vertex_count(vertices.Size()),
        m_buffer_index_count(indices.Size()),
        m_uint32_index(uint32_index)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        m_submeshes = submeshes;
        if (m_submeshes.Empty())
        {
            m_submeshes.Add(Submesh({ 0, indices.Size() }));
        }
        
        filament::backend::BufferUsage usage;
        if (dynamic)
        {
            usage = filament::backend::BufferUsage::DYNAMIC;
        }
        else
        {
            usage = filament::backend::BufferUsage::STATIC;
        }
        
        int sizes[] = {
            sizeof(Vertex::vertex), sizeof(Vertex::color), sizeof(Vertex::uv), sizeof(Vertex::uv2),
            sizeof(Vertex::normal), sizeof(Vertex::tangent), sizeof(Vertex::bone_weight), sizeof(Vertex::bone_indices)
            
        };
        filament::backend::ElementType types[] = {
            filament::backend::ElementType::FLOAT3,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT2,
            filament::backend::ElementType::FLOAT2,
            filament::backend::ElementType::FLOAT3,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT4
        };
        int offset = 0;
        
        filament::backend::AttributeArray attributes;
        for (int i = 0; i < (int) Shader::AttributeLocation::Count; ++i)
        {
            attributes[i].offset = offset;
            attributes[i].stride = sizeof(Vertex);
            attributes[i].buffer = 0;
            attributes[i].type = types[i];
            attributes[i].flags = 0;
            
            offset += sizes[i];
        }
        
        m_vb = driver.createVertexBuffer(1, (uint8_t) Shader::AttributeLocation::Count, m_vertex_count, attributes, usage);

        filament::backend::ElementType index_type;
        if (uint32_index)
        {
            index_type = filament::backend::ElementType::UINT;
        }
        else
        {
            index_type = filament::backend::ElementType::USHORT;
        }
        
        m_ib = driver.createIndexBuffer(index_type, m_index_count, usage);
        
        Mesh::Update(vertices, indices, submeshes);
    }
    
    Mesh::~Mesh()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        for (int i = 0; i < m_primitives.Size(); ++i)
        {
            driver.destroyRenderPrimitive(m_primitives[i]);
        }
        m_primitives.Clear();
        driver.destroyIndexBuffer(m_ib);
        driver.destroyVertexBuffer(m_vb);
    }

    void Mesh::Update(const Vector<Vertex>& vertices, const Vector<unsigned int>& indices, const Vector<Submesh>& submeshes)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
        assert(vertices.Size() <= m_buffer_vertex_count);
        assert(indices.Size() <= m_buffer_index_count);
        
        m_vertex_count = vertices.Size();
        m_index_count = indices.Size();
        m_submeshes = submeshes;
        if (m_submeshes.Empty())
        {
            m_submeshes.Add(Submesh({ 0, indices.Size() }));
        }
        
        void* buffer = Memory::Alloc<void>(vertices.SizeInBytes());
        Memory::Copy(buffer, vertices.Bytes(), vertices.SizeInBytes());
        driver.updateVertexBuffer(m_vb, 0, filament::backend::BufferDescriptor(buffer, vertices.SizeInBytes(), FreeBufferCallback), 0);
    
        if (m_uint32_index)
        {
            buffer = Memory::Alloc<void>(indices.SizeInBytes());
            Memory::Copy(buffer, indices.Bytes(), indices.SizeInBytes());
            driver.updateIndexBuffer(m_ib, filament::backend::BufferDescriptor(buffer, indices.SizeInBytes(), FreeBufferCallback), 0);
        }
        else
        {
            int size = sizeof(unsigned short) * indices.Size();
            unsigned short* indices_uint16 = Memory::Alloc<unsigned short>(size);
            for (int i = 0; i < indices.Size(); ++i)
            {
                indices_uint16[i] = indices[i];
            }
            driver.updateIndexBuffer(m_ib, filament::backend::BufferDescriptor(indices_uint16, size, FreeBufferCallback), 0);
        }
        
        for (int i = 0; i < m_primitives.Size(); ++i)
        {
            driver.destroyRenderPrimitive(m_primitives[i]);
        }
        m_primitives.Clear();
        
        uint32_t enabled_attributes =
            (1 << (int) Shader::AttributeLocation::Vertex) |
            (1 << (int) Shader::AttributeLocation::Color) |
            (1 << (int) Shader::AttributeLocation::Texcoord) |
            (1 << (int) Shader::AttributeLocation::Texcoord2) |
            (1 << (int) Shader::AttributeLocation::Normal) |
            (1 << (int) Shader::AttributeLocation::Tangent) |
            (1 << (int) Shader::AttributeLocation::BlendWeight) |
            (1 << (int) Shader::AttributeLocation::BlendIndices);
        
        m_primitives.Resize(m_submeshes.Size());
        for (int i = 0; i < m_submeshes.Size(); ++i)
        {
            m_primitives[i] = driver.createRenderPrimitive();
            
            driver.setRenderPrimitiveBuffer(m_primitives[i], m_vb, m_ib, enabled_attributes);
            driver.setRenderPrimitiveRange(m_primitives[i], filament::backend::PrimitiveType::TRIANGLES, m_submeshes[i].index_first, 0, m_vertex_count - 1, m_submeshes[i].index_count);
        }
    }
}
