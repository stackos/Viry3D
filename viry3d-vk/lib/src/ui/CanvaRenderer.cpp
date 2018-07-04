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
#include "graphics/Mesh.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	CanvaRenderer::CanvaRenderer():
		m_canvas_dirty(true)
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

Output(0) vec2 v_uv;
Output(1) vec4 v_color;

void main()
{
	gl_Position = a_pos * buf_0_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;
	v_uv = a_uv;
	v_color = a_color;

	vulkan_convert();
}
)";
			String fs = R"(
precision highp float;
      
UniformTexture(0, 1) uniform sampler2D u_texture;

Input(0) vec2 v_uv;
Input(1) vec4 v_color;

Output(0) vec4 o_frag;

void main()
{
    o_frag = v_color;//texture(u_texture, v_uv) * 
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

	CanvaRenderer::~CanvaRenderer()
	{

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

	void CanvaRenderer::GetIndexRange(int& index_offset, int& index_count) const
	{
		index_offset = 0;
		index_count = 0;

		if (m_mesh)
		{
			index_offset = 0;
			index_count = m_mesh->GetIndexCount();
		}
	}

	void CanvaRenderer::Update()
	{
		Renderer::Update();

		if (m_canvas_dirty)
		{
			m_canvas_dirty = false;
			this->UpdateCanvas();
            this->UpdateProjectionMatrix();
			this->MarkInstanceCmdDirty();
		}
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

	void CanvaRenderer::UpdateCanvas()
	{
        Vector<Vertex> vertices;
        Vector<unsigned short> indices;

        for (int i = 0; i < m_views.Size(); ++i)
        {
            m_views[i]->UpdateLayout();
            m_views[i]->FillVertices(vertices, indices);
        }

        if (vertices.Size() > 0 && indices.Size() > 0)
        {
            m_mesh = RefMake<Mesh>(vertices, indices);
        }
        else
        {
            m_mesh.reset();
        }
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
        auto projection_matrix = Matrix4x4::Ortho(-plane_w / 2, plane_w / 2, bottom, top, -1, 1);
        this->GetMaterial()->SetMatrix("u_projection_matrix", projection_matrix);
    }
}
