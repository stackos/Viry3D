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

#include "CanvaRenderer.h"
#include "View.h"
#include "Debug.h"
#include "graphics/Mesh.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "graphics/BufferObject.h"
#include "graphics/Image.h"
#include "memory/Memory.h"
#include "container/List.h"

#define ATLAS_SIZE 2048
#define PADDING_SIZE 1

namespace Viry3D
{
	CanvaRenderer::CanvaRenderer():
		m_canvas_dirty(true),
        m_atlas_array_size(0)
	{
		this->CreateMaterial();
        this->NewAtlasTextureLayer();
	}

	CanvaRenderer::~CanvaRenderer()
	{
        for (int i = 0; i < m_atlas_tree.Size(); ++i)
        {
            this->ReleaseAtlasTreeNode(m_atlas_tree[i]);
            delete m_atlas_tree[i];
        }
        m_atlas_tree.Clear();

        if (m_draw_buffer)
        {
            m_draw_buffer->Destroy(Display::Instance()->GetDevice());
            m_draw_buffer.reset();
        }
	}

    void CanvaRenderer::ReleaseAtlasTreeNode(AtlasTreeNode* node)
    {
        for (int i = 0; i < node->children.Size(); ++i)
        {
            this->ReleaseAtlasTreeNode(node->children[i]);
            delete node->children[i];
        }
        node->children.Clear();
    }

    void CanvaRenderer::CreateMaterial()
    {
        auto shader = Shader::Find("UI");
        if (!shader)
        {
            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_model_matrix;
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
} buf_0_0;

Input(0) vec4 a_pos;
Input(1) vec4 a_color;
Input(2) vec2 a_uv;
Input(3) vec2 a_uv2;

Output(0) vec3 v_uv;
Output(1) vec4 v_color;

void main()
{
	gl_Position = a_pos * buf_0_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;
	v_uv = vec3(a_uv, a_uv2.x);
	v_color = a_color;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
precision lowp sampler2DArray;

UniformTexture(0, 1) uniform sampler2DArray u_texture;

Input(0) vec3 v_uv;
Input(1) vec4 v_color;

Output(0) vec4 o_frag;

void main()
{
    o_frag = texture(u_texture, v_uv) * v_color;
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;
            render_state.blend = RenderState::Blend::On;
            render_state.srcBlendMode = RenderState::BlendMode::SrcAlpha;
            render_state.dstBlendMode = RenderState::BlendMode::OneMinusSrcAlpha;
            render_state.queue = (int) RenderState::Queue::Transparent;

            shader = RefMake<Shader>(
                vs,
                Vector<String>(),
                fs,
                Vector<String>(),
                render_state);
            Shader::AddCache("UI", shader);
        }

        auto material = RefMake<Material>(shader);
        material->SetMatrix("u_model_matrix", Matrix4x4::Identity());

        auto view_matrix = Matrix4x4::LookTo(
            Vector3(0, 0, 0),
            Vector3(0, 0, 1),
            Vector3(0, 1, 0));
        material->SetMatrix("u_view_matrix", view_matrix);

        this->SetMaterial(material);
    }

    void CanvaRenderer::NewAtlasTextureLayer()
    {
        ByteBuffer buffer(ATLAS_SIZE * ATLAS_SIZE * 4);
        Memory::Set(&buffer[0], 0, buffer.Size());

        if (!m_atlas)
        {
            m_atlas_array_size = 1;

            Vector<ByteBuffer> pixels(m_atlas_array_size, buffer);

            m_atlas = Texture::CreateTexture2DArrayFromMemory(
                pixels,
                ATLAS_SIZE,
                ATLAS_SIZE,
                m_atlas_array_size,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                false,
                true);
        }
        else
        {
            int new_array_size = m_atlas_array_size + 1;

            Vector<ByteBuffer> pixels(new_array_size, buffer);

            Ref<Texture> new_atlas = Texture::CreateTexture2DArrayFromMemory(
                pixels,
                ATLAS_SIZE,
                ATLAS_SIZE,
                new_array_size,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                false,
                true);

            for (int i = 0; i < m_atlas_array_size; ++i)
            {
                new_atlas->CopyTexture(
                    m_atlas,
                    i, 0,
                    0, 0,
                    i, 0,
                    0, 0,
                    ATLAS_SIZE, ATLAS_SIZE);
            }

            m_atlas = new_atlas;
            m_atlas_array_size = new_array_size;
        }

        AtlasTreeNode* layer = new AtlasTreeNode();
        layer->x = 0;
        layer->y = 0;
        layer->w = ATLAS_SIZE;
        layer->h = ATLAS_SIZE;
        layer->layer = m_atlas_tree.Size();
        m_atlas_tree.Add(layer);

        this->GetMaterial()->SetTexture("u_texture", m_atlas);
    }

	Ref<BufferObject> CanvaRenderer::GetVertexBuffer() const
	{
		Ref<BufferObject> buffer;

		if (m_mesh)
		{
			buffer = m_mesh->GetVertexBuffer();
		}

		return buffer;
	}

	Ref<BufferObject> CanvaRenderer::GetIndexBuffer() const
	{
		Ref<BufferObject> buffer;

		if (m_mesh)
		{
			buffer = m_mesh->GetIndexBuffer();
		}

		return buffer;
	}

	void CanvaRenderer::Update()
	{
		if (m_canvas_dirty)
		{
			m_canvas_dirty = false;
            this->UpdateProjectionMatrix();
            this->UpdateCanvas();
		}

        Renderer::Update();
	}

    void CanvaRenderer::OnResize(int width, int height)
    {
        this->MarkCanvasDirty();
    }

	void CanvaRenderer::AddView(const Ref<View>& view)
	{
		m_views.Add(view);
		view->OnAddToCanvas(this);
		this->MarkCanvasDirty();
	}

	void CanvaRenderer::RemoveView(const Ref<View>& view)
	{
		m_views.Remove(view);
		view->OnRemoveFromCanvas(this);
		this->MarkCanvasDirty();
	}

	void CanvaRenderer::MarkCanvasDirty()
	{
		m_canvas_dirty = true;
	}

    void CanvaRenderer::UpdateProjectionMatrix()
    {
        auto camera = this->GetCamera();
        int target_width = camera->GetTargetWidth();
        int target_height = camera->GetTargetHeight();
        float ortho_size = target_height / 2.0f;
        float top = ortho_size;
        float bottom = -ortho_size;
        float plane_h = ortho_size * 2;
        float plane_w = plane_h * target_width / target_height;
        auto projection_matrix = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, -1000, 1000);

        this->GetMaterial()->SetMatrix("u_projection_matrix", projection_matrix);
    }

    void CanvaRenderer::UpdateCanvas()
    {
        Vector<ViewMesh> meshes;
        
        for (int i = 0; i < m_views.Size(); ++i)
        {
            m_views[i]->UpdateLayout();
            m_views[i]->FillMeshes(meshes);
        }

        List<ViewMesh*> mesh_list;

        for (int i = 0; i < meshes.Size(); ++i)
        {
            mesh_list.AddLast(&meshes[i]);
        }

        mesh_list.Sort([](const ViewMesh* a, const ViewMesh* b) {
            if (a->texture->GetWidth() == b->texture->GetWidth())
            {
                return a->texture->GetHeight() > b->texture->GetHeight();
            }
            else
            {
                return a->texture->GetWidth() > b->texture->GetWidth();
            }
        });

        bool atlas_updated = false;
        for (auto i : mesh_list)
        {
            bool updated;
            this->UpdateAtlas(*i, updated);

            if (updated)
            {
                atlas_updated = true;
            }
        }

        Vector<Vertex> vertices;
        Vector<unsigned short> indices;

        for (const auto& i : meshes)
        {
            if (i.vertices.Size() > 0 && i.indices.Size() > 0 && i.texture)
            {
                int index_offset = vertices.Size();

                vertices.AddRange(i.vertices);

                for (int j = 0; j < i.indices.Size(); ++j)
                {
                    indices.Add(index_offset + i.indices[j]);
                }
            }
        }

        bool draw_buffer_dirty = false;

        if (!m_mesh)
        {
            if (indices.Size() > 0)
            {
                draw_buffer_dirty = true;
            }
        }
        else
        {
            if (indices.Size() != m_mesh->GetIndexCount())
            {
                draw_buffer_dirty = true;
            }
        }

        if (vertices.Size() > 0 && indices.Size() > 0)
        {
            if (!m_mesh || vertices.Size() > m_mesh->GetVertexCount() || indices.Size() > m_mesh->GetIndexCount())
            {
                m_mesh = RefMake<Mesh>(vertices, indices);
                this->MarkInstanceCmdDirty();
            }
            else
            {
                m_mesh->Update(vertices, indices);
            }
        }
        else
        {
            m_mesh.reset();
        }

        if (draw_buffer_dirty)
        {
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
        }

        /*
        // test output atlas texture
        if (atlas_updated)
        {
            ByteBuffer pixels;

            if (m_atlas_array_size > 0)
            {
                m_atlas->CopyToMemory(pixels, 0, 0);
                Image::EncodeToPNG("atlas0.png", pixels, ATLAS_SIZE, ATLAS_SIZE, 32);
            }

            if (m_atlas_array_size > 1)
            {
                m_atlas->CopyToMemory(pixels, 1, 0);
                Image::EncodeToPNG("atlas1.png", pixels, ATLAS_SIZE, ATLAS_SIZE, 32);
            }

            if (m_atlas_array_size > 2)
            {
                m_atlas->CopyToMemory(pixels, 2, 0);
                Image::EncodeToPNG("atlas2.png", pixels, ATLAS_SIZE, ATLAS_SIZE, 32);
            }
        }
        */
    }

    void CanvaRenderer::UpdateAtlas(ViewMesh& mesh, bool& updated)
    {
        assert(mesh.texture->GetWidth() <= ATLAS_SIZE - PADDING_SIZE && mesh.texture->GetHeight() <= ATLAS_SIZE - PADDING_SIZE);

        AtlasTreeNode* node = nullptr;

        AtlasTreeNode** node_ptr;
        if (m_atlas_cache.TryGet(mesh.texture.get(), &node_ptr))
        {
            node = *node_ptr;

            updated = false;
        }
        else
        {
            for (int i = 0; i < m_atlas_tree.Size(); ++i)
            {
                node = this->FindAtlasTreeNodeToInsert(mesh.texture->GetWidth(), mesh.texture->GetHeight(), m_atlas_tree[i]);
                if (node)
                {
                    break;
                }
            }

            if (node == nullptr)
            {
                this->NewAtlasTextureLayer();

                node = this->FindAtlasTreeNodeToInsert(mesh.texture->GetWidth(), mesh.texture->GetHeight(), m_atlas_tree[m_atlas_tree.Size() - 1]);
            }

            assert(node);

            // split node
            AtlasTreeNode* left = new AtlasTreeNode();
            left->x = node->x + mesh.texture->GetWidth() + PADDING_SIZE;
            left->y = node->y;
            left->w = node->w - mesh.texture->GetWidth() - PADDING_SIZE;
            left->h = mesh.texture->GetHeight();
            left->layer = node->layer;

            AtlasTreeNode* right = new AtlasTreeNode();
            right->x = node->x;
            right->y = node->y + mesh.texture->GetHeight() + PADDING_SIZE;
            right->w = node->w;
            right->h = node->h - mesh.texture->GetHeight() - PADDING_SIZE;
            right->layer = node->layer;

            node->w = mesh.texture->GetWidth();
            node->h = mesh.texture->GetHeight();
            node->children.Resize(2);
            node->children[0] = left;
            node->children[1] = right;

            // copy texture to atlas
            m_atlas->CopyTexture(
                mesh.texture,
                0, 0,
                0, 0,
                node->layer, 0,
                node->x, node->y,
                node->w, node->h);

            // add cache
            m_atlas_cache.Add(mesh.texture.get(), node);

            updated = true;
        }

        // update uv
        Vector2 uv_offset(node->x / (float) ATLAS_SIZE, node->y / (float) ATLAS_SIZE);
        Vector2 uv_scale(node->w / (float) ATLAS_SIZE, node->h / (float) ATLAS_SIZE);

        for (int i = 0; i < mesh.vertices.Size(); ++i)
        {
            mesh.vertices[i].uv.x = mesh.vertices[i].uv.x * uv_scale.x + uv_offset.x;
            mesh.vertices[i].uv.y = mesh.vertices[i].uv.y * uv_scale.y + uv_offset.y;
            mesh.vertices[i].uv2.x = (float) node->layer;
        }
    }

    AtlasTreeNode* CanvaRenderer::FindAtlasTreeNodeToInsert(int w, int h, AtlasTreeNode* node)
    {
        if (node->children.Size() == 0)
        {
            if (node->w - PADDING_SIZE >= w && node->h - PADDING_SIZE >= h)
            {
                return node;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            AtlasTreeNode* left = this->FindAtlasTreeNodeToInsert(w, h, node->children[0]);
            if (left)
            {
                return left;
            }
            else
            {
                return this->FindAtlasTreeNodeToInsert(w, h, node->children[1]);
            }
        }
    }
}
