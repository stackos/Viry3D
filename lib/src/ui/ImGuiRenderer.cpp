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

#include "ImGuiRenderer.h"
#include "time/Time.h"
#include "graphics/Texture.h"
#include "graphics/Mesh.h"
#include "graphics/BufferObject.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "Input.h"
#include "imgui/imgui.h"

namespace Viry3D
{
    ImGuiRenderer::ImGuiRenderer()
    {
        ImGui::CreateContext();

        this->CreateMaterial();
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        ImGui::DestroyContext();

#if VR_VULKAN
        if (m_draw_buffer)
        {
            m_draw_buffer->Destroy(Display::Instance()->GetDevice());
            m_draw_buffer.reset();
        }
#endif
    }

    Ref<BufferObject> ImGuiRenderer::GetVertexBuffer() const
    {
        Ref<BufferObject> buffer;

        if (m_mesh)
        {
            buffer = m_mesh->GetVertexBuffer();
        }

        return buffer;
    }

    Ref<BufferObject> ImGuiRenderer::GetIndexBuffer() const
    {
        Ref<BufferObject> buffer;

        if (m_mesh)
        {
            buffer = m_mesh->GetIndexBuffer();
        }

        return buffer;
    }

    void ImGuiRenderer::UpdateDrawBuffer()
    {
#if VR_VULKAN
        VkDrawIndexedIndirectCommand draw;
        if (m_mesh)
        {
            draw.indexCount = m_mesh->GetIndexCount();
        }
        else
        {
            draw.indexCount = 0;
        }
        draw.instanceCount = 1;
        draw.firstIndex = 0;
        draw.vertexOffset = 0;
        draw.firstInstance = 0;

        if (!m_draw_buffer)
        {
            m_draw_buffer = Display::Instance()->CreateBuffer(&draw, sizeof(draw), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, false, VK_FORMAT_UNDEFINED);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_draw_buffer, 0, &draw, sizeof(draw));
        }
#elif VR_GLES
        m_draw_buffer.first_index = 0;
        if (m_mesh)
        {
            m_draw_buffer.index_count = m_mesh->GetIndexCount();
        }
        else
        {
            m_draw_buffer.index_count = 0;
        }
#endif
    }

    void ImGuiRenderer::CreateMaterial()
    {
        auto shader = Shader::Find("IMGUI");
        if (!shader)
        {
#if VR_VULKAN
            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
};

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
};

Input(0) vec3 a_pos;
Input(1) vec4 a_color;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;
Output(1) vec4 v_color;

void main()
{
	gl_Position = vec4(a_pos, 1.0) * u_model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = a_uv;
	v_color = a_color;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;

UniformTexture(0, 1) uniform sampler2D u_texture;

UniformBuffer(0, 2) uniform UniformBuffer02
{
	vec4 u_color; 
};

Input(0) vec2 v_uv;
Input(1) vec4 v_color;

Output(0) vec4 o_frag;

void main()
{
    o_frag = texture(u_texture, v_uv) * v_color * u_color;
}
)";
#elif VR_GLES
            String vs = R"(
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;

attribute vec3 a_pos;
attribute vec4 a_color;
attribute vec2 a_uv;

varying vec2 v_uv;
varying vec4 v_color;

void main()
{
	gl_Position = vec4(a_pos, 1.0) * u_model_matrix * u_view_matrix * u_projection_matrix;
    v_uv = a_uv;
	v_color = a_color;
}
)";
            String fs = R"(
precision highp float;

uniform sampler2D u_texture;
uniform vec4 u_color;

varying vec2 v_uv;
varying vec4 v_color;

void main()
{
    gl_FragColor = texture2D(u_texture, v_uv) * v_color * u_color;
}
)";
#endif
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;
            render_state.blend = RenderState::Blend::On;
            render_state.srcBlendMode = RenderState::BlendMode::SrcAlpha;
            render_state.dstBlendMode = RenderState::BlendMode::OneMinusSrcAlpha;
            render_state.queue = (int) RenderState::Queue::Transparent;

            shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                "",
                Vector<String>(),
                fs,
                render_state);
            Shader::AddCache("IMGUI", shader);
        }

        auto material = RefMake<Material>(shader);
        material->SetColor("u_color", Color(1, 1, 1, 1));

        this->SetMaterial(material);
    }

    void ImGuiRenderer::Update()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float) Display::Instance()->GetWidth(), (float) Display::Instance()->GetHeight());
        io.DeltaTime = Time::GetDeltaTime();

        // input
        for (int i = 0; i < 3; ++i)
        {
            if (Input::GetMouseButtonDown(i))
            {
                io.MouseDown[i] = true;
            }
            if (Input::GetMouseButtonUp(i))
            {
                io.MouseDown[i] = false;
            }
        }
        const auto& pos = Input::GetMousePosition();
        io.MousePos = ImVec2(pos.x, Display::Instance()->GetHeight() - pos.y - 1);
        io.MouseWheel += Input::GetMouseScrollWheel();
        // key
        // TODO

        if (!m_font_texture)
        {
            unsigned char* font_tex_pixels;
            int font_tex_width;
            int font_tex_height;
            io.Fonts->GetTexDataAsRGBA32(&font_tex_pixels, &font_tex_width, &font_tex_height);

            ByteBuffer pixels(font_tex_pixels, font_tex_width * font_tex_height * 4);
            m_font_texture = Texture::CreateTexture2DFromMemory(
                pixels,
                font_tex_width,
                font_tex_height,
                TextureFormat::R8G8B8A8,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge,
                false,
                false,
                false);

            this->GetMaterial()->SetTexture("u_texture", m_font_texture);
        }

        ImGui::NewFrame();
        if (m_draw)
        {
            m_draw();
        }
        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data->Valid)
        {
            // update matrix
            this->GetCamera()->SetNearClip(-1000);
            this->GetCamera()->SetFarClip(1000);
            this->GetCamera()->SetOrthographic(true);
            this->GetCamera()->SetOrthographicSize(this->GetCamera()->GetTargetHeight() / 2.0f);

            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
            auto projection_matrix = Matrix4x4::Ortho(L, R, B, T, this->GetCamera()->GetNearClip(), this->GetCamera()->GetFarClip());
            this->GetCamera()->SetProjectionMatrixExternal(projection_matrix);
            this->GetCamera()->SetProjectionUniform(this->GetMaterial());

            // update mesh
            Vector<Vertex> vertices(draw_data->TotalVtxCount);
            Vector<unsigned short> indices;
            int vertex_index = 0;

            for (int i = 0; i < draw_data->CmdListsCount; ++i)
            {
                auto cmd = draw_data->CmdLists[i];
                int index_index = 0;

                for (int j = 0; j < cmd->CmdBuffer.size(); ++j)
                {
                    for (unsigned int k = 0; k < cmd->CmdBuffer[j].ElemCount; ++k)
                    {
                        indices.Add(cmd->IdxBuffer[k + index_index] + vertex_index);
                    }
                    index_index += cmd->CmdBuffer[j].ElemCount;

                    // clip rect
                    // TODO
                }

                for (int j = 0; j < cmd->VtxBuffer.size(); ++j)
                {
                    const auto& v = cmd->VtxBuffer[j];
                    vertices[vertex_index].vertex = Vector3(v.pos.x, v.pos.y, 0);
                    vertices[vertex_index].uv = Vector2(v.uv.x, v.uv.y);
                    uint32_t r = (v.col >> 24) & 0xff;
                    uint32_t g = (v.col >> 16) & 0xff;
                    uint32_t b = (v.col >> 8) & 0xff;
                    uint32_t a = (v.col >> 0) & 0xff;
                    vertices[vertex_index].color = Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                    ++vertex_index;
                }
            }

            assert(vertex_index == vertices.Size());

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
                    m_mesh = RefMake<Mesh>(vertices, indices, Vector<Mesh::Submesh>(), true);

#if VR_VULKAN
                    this->MarkInstanceCmdDirty();
#endif
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
                m_draw_buffer_dirty = true;
            }
        }

        Renderer::Update();
    }
}
