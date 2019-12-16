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
#include "Texture.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "memory/Memory.h"

namespace Viry3D
{
	Ref<Mesh> Mesh::m_shared_quad_mesh;

	void Mesh::Init()
	{
	
	}

	void Mesh::Done()
	{
		m_shared_quad_mesh.reset();
	}

	const Ref<Mesh>& Mesh::GetSharedQuadMesh()
	{
		if (!m_shared_quad_mesh)
		{
			Vector<Mesh::Vertex> vertices(4);
			vertices[0].vertex = Vector3(-1, 1, 0);
			vertices[1].vertex = Vector3(-1, -1, 0);
			vertices[2].vertex = Vector3(1, -1, 0);
			vertices[3].vertex = Vector3(1, 1, 0);
			vertices[0].uv = Vector2(0, 0);
			vertices[1].uv = Vector2(0, 1);
			vertices[2].uv = Vector2(1, 1);
			vertices[3].uv = Vector2(1, 0);
			Vector<unsigned int> indices = {
				0, 1, 2, 0, 2, 3
			};
			m_shared_quad_mesh = RefMake<Mesh>(std::move(vertices), std::move(indices));
		}

		return m_shared_quad_mesh;
	}

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
            Vector<BlendShape>* blend_shapes = new Vector<BlendShape>();
            
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
                (*vertices)[i].bone_weights = ms.Read<Vector4>();
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
            
            int blend_shape_count = ms.Read<int>();
            if (blend_shape_count > 0)
            {
                blend_shapes->Resize(blend_shape_count);

                for (int i = 0; i < blend_shape_count; ++i)
                {
                    auto& shape = (*blend_shapes)[i];
                    
                    int string_size = ms.Read<int>();
                    String shape_name = ms.ReadString(string_size);
                    int frame_count = ms.Read<int>();
                    
                    shape.name = shape_name;
                    auto& frame = shape.frame;

                    if (frame_count > 0)
                    {
                        frame.vertices.Resize(vertex_count, Vector3::Zero());
                        frame.normals.Resize(normal_count, Vector3::Zero());
                        frame.tangents.Resize(tangent_count, Vector3::Zero());
                    }

                    for (int j = 0; j < frame_count; ++j)
                    {
                        float weight = ms.Read<float>() / 100.0f;

                        for (int k = 0; k < vertex_count; ++k)
                        {
                            Vector3 vertex = ms.Read<Vector3>();
                            frame.vertices[k] += vertex * weight;
                        }

                        for (int k = 0; k < normal_count; ++k)
                        {
                            Vector3 normal = ms.Read<Vector3>();
                            frame.normals[k] += normal * weight;
                        }

                        for (int k = 0; k < tangent_count; ++k)
                        {
                            Vector3 tangent = ms.Read<Vector3>();
                            frame.tangents[k] += tangent * weight;
                        }
                    }
                }
            }
            
            Vector3 bounds_center = ms.Read<Vector3>();
            Vector3 bounds_size = ms.Read<Vector3>();

            mesh = RefMake<Mesh>(std::move(*vertices), std::move(*indices), *submeshes);
            mesh->SetName(mesh_name);
            mesh->SetBindposes(std::move(*bindposes));
            mesh->SetBlendShapes(std::move(*blend_shapes));
            mesh->m_bounds = Bounds(bounds_center - bounds_size / 2, bounds_center + bounds_size / 2);

            // create blendshape texture
            if (blend_shape_count > 0 && Texture::SelectFormat({ TextureFormat::R32G32B32A32F }, false) != TextureFormat::None)
            {
                int vector_count = vertex_count * blend_shape_count + normal_count * blend_shape_count + tangent_count * blend_shape_count;
                const int blend_shape_texture_width = 2048;
                int blend_shape_texture_height = vector_count / blend_shape_texture_width;
                if (vector_count % blend_shape_texture_width != 0)
                {
                    blend_shape_texture_height += 1;
                }
                assert(blend_shape_texture_height <= 2048);
                vector_count = blend_shape_texture_height * blend_shape_texture_width;

                ByteBuffer blend_shape_texture_buffer(vector_count * sizeof(Vector4));
                Memory::Zero(blend_shape_texture_buffer.Bytes(), blend_shape_texture_buffer.Size());
                Vector4* pvector = (Vector4*) blend_shape_texture_buffer.Bytes();
                for (int i = 0; i < blend_shape_count; ++i)
                {
                    for (int j = 0; j < vertex_count; ++j)
                    {
                        pvector[i * vertex_count + j] = mesh->m_blend_shapes[i].frame.vertices[j];
                    }

                    for (int j = 0; j < normal_count; ++j)
                    {
                        pvector[vertex_count * blend_shape_count + i * normal_count + j] = mesh->m_blend_shapes[i].frame.normals[j];
                    }

                    for (int j = 0; j < tangent_count; ++j)
                    {
                        pvector[vertex_count * blend_shape_count + normal_count * blend_shape_count + i * tangent_count + j] = mesh->m_blend_shapes[i].frame.tangents[j];
                    }
                }
                
                mesh->m_blend_shape_texture = Texture::CreateTexture2DFromMemory(
                    blend_shape_texture_buffer,
                    blend_shape_texture_width,
                    blend_shape_texture_height,
                    TextureFormat::R32G32B32A32F,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge,
                    false);
            }

            delete vertices;
            delete indices;
            delete submeshes;
            delete bindposes;
            delete blend_shapes;
        }
        else
        {
            Log("mesh file not exist: %s", path.CString());
        }

        return mesh;
    }

    Mesh::Mesh(Vector<Vertex>&& vertices, Vector<unsigned int>&& indices, const Vector<Submesh>& submeshes, bool uint32_index, bool dynamic):
        m_buffer_vertex_count(vertices.Size()),
        m_buffer_index_count(indices.Size()),
        m_uint32_index(uint32_index),
		m_enabled_attributes(0)
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
            sizeof(Vertex::normal), sizeof(Vertex::tangent), sizeof(Vertex::bone_weights), sizeof(Vertex::bone_indices)
        };
        filament::backend::ElementType types[] = {
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT2,
            filament::backend::ElementType::FLOAT2,
            filament::backend::ElementType::FLOAT3,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT4,
            filament::backend::ElementType::FLOAT4
        };
        int offset = 0;
        
        for (int i = 0; i < (int) Shader::AttributeLocation::Count; ++i)
        {
			m_attributes[i].offset = offset;
			m_attributes[i].stride = sizeof(Vertex);
			m_attributes[i].buffer = 0;
			m_attributes[i].type = types[i];
			m_attributes[i].flags = 0;
            
            offset += sizes[i];
        }
        
        m_vb = driver.createVertexBuffer(1, (uint8_t) Shader::AttributeLocation::Count, vertices.Size(), m_attributes, usage);

        filament::backend::ElementType index_type;
        if (uint32_index)
        {
            index_type = filament::backend::ElementType::UINT;
        }
        else
        {
            index_type = filament::backend::ElementType::USHORT;
        }
        
        m_ib = driver.createIndexBuffer(index_type, indices.Size(), usage);
        
        Mesh::Update(std::move(vertices), std::move(indices), submeshes);
    }
    
    Mesh::~Mesh()
    {
        auto& driver = Engine::Instance()->GetDriverApi();
        
		driver.destroyVertexBuffer(m_vb);
		m_vb.clear();

		driver.destroyIndexBuffer(m_ib);
		m_ib.clear();

		for (int i = 0; i < m_primitives.Size(); ++i)
		{
			driver.destroyRenderPrimitive(m_primitives[i]);
			m_primitives[i].clear();
		}
		m_primitives.Clear();
    }

    void Mesh::Update(Vector<Vertex>&& vertices, Vector<unsigned int>&& indices, const Vector<Submesh>& submeshes)
    {
        auto& driver = Engine::Instance()->GetDriverApi();
     
        m_vertices = std::move(vertices);
        m_indices = std::move(indices);

        for (int i = 0; i < m_vertices.Size(); ++i)
        {
            m_vertices[i].vertex.w = (float) i;
        }
        
        assert(m_vertices.Size() <= m_buffer_vertex_count);
        assert(m_indices.Size() <= m_buffer_index_count);
        
        m_submeshes = submeshes;
        if (m_submeshes.Empty())
        {
            m_submeshes.Add(Submesh({ 0, m_indices.Size() }));
        }
        
        void* buffer = Memory::Alloc<void>(m_vertices.SizeInBytes());
        Memory::Copy(buffer, m_vertices.Bytes(), m_vertices.SizeInBytes());
        driver.updateVertexBuffer(m_vb, 0, filament::backend::BufferDescriptor(buffer, m_vertices.SizeInBytes(), FreeBufferCallback), 0);
    
        if (m_uint32_index)
        {
            buffer = Memory::Alloc<void>(m_indices.SizeInBytes());
            Memory::Copy(buffer, m_indices.Bytes(), m_indices.SizeInBytes());
            driver.updateIndexBuffer(m_ib, filament::backend::BufferDescriptor(buffer, m_indices.SizeInBytes(), FreeBufferCallback), 0);
        }
        else
        {
            int size = sizeof(unsigned short) * m_indices.Size();
            unsigned short* indices_uint16 = Memory::Alloc<unsigned short>(size);
            for (int i = 0; i < m_indices.Size(); ++i)
            {
                indices_uint16[i] = m_indices[i];
            }
            driver.updateIndexBuffer(m_ib, filament::backend::BufferDescriptor(indices_uint16, size, FreeBufferCallback), 0);
        }
        
		m_enabled_attributes =
            (1 << (int) Shader::AttributeLocation::Vertex) |
            (1 << (int) Shader::AttributeLocation::Color) |
            (1 << (int) Shader::AttributeLocation::UV) |
            (1 << (int) Shader::AttributeLocation::UV2) |
            (1 << (int) Shader::AttributeLocation::Normal) |
            (1 << (int) Shader::AttributeLocation::Tangent) |
            (1 << (int) Shader::AttributeLocation::BoneWeights) |
            (1 << (int) Shader::AttributeLocation::BoneIndices);
        
        for (int i = 0; i < m_primitives.Size(); ++i)
        {
            driver.destroyRenderPrimitive(m_primitives[i]);
			m_primitives[i].clear();
        }
        m_primitives.Clear();
        
        m_primitives.Resize(m_submeshes.Size());
        for (int i = 0; i < m_primitives.Size(); ++i)
        {
            m_primitives[i] = driver.createRenderPrimitive();
            
            driver.setRenderPrimitiveBuffer(m_primitives[i], m_vb, m_ib, m_enabled_attributes);
            driver.setRenderPrimitiveRange(m_primitives[i], filament::backend::PrimitiveType::TRIANGLES, m_submeshes[i].index_first, 0, m_vertices.Size() - 1, m_submeshes[i].index_count);
        }
    }

    void Mesh::SetBlendShapes(Vector<BlendShape>&& blend_shapes)
    {
        m_blend_shapes = std::move(blend_shapes);
    }
}
