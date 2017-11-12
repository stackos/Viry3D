/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Graphics.h"
#include "Application.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "FrameBuffer.h"
#include "RenderPass.h"
#include "RenderTexture.h"
#include "DescriptorSet.h"

namespace Viry3D
{
	int Graphics::draw_call = 0;
	Ref<Display> Graphics::m_display;
	Ref<Mesh> Graphics::m_blit_mesh;
	Vector<Ref<Material>> Graphics::m_blit_materials;
	Vector<Ref<RenderPass>> Graphics::m_blit_render_passes;
	Ref<DescriptorSet> Graphics::m_draw_descriptor_set;
	Ref<UniformBuffer> Graphics::m_draw_descriptor_set_buffer;
	CullFace Graphics::m_global_cull_face = CullFace::NoSet;

	void Graphics::Init(int width, int height, int fps)
	{
		m_display = RefMake<Display>();
		m_display->Init(width, height, fps);

		Application::RunTaskInPreLoop(RunLoop::Task([] {
			m_display->ProcessSystemEvents();
		}, false));
	}

	void Graphics::OnResize(int width, int height)
	{
		m_blit_render_passes.Clear();
		Camera::OnResize(width, height);
		m_display->OnResize(width, height);
	}

	void Graphics::OnPause()
	{
		m_blit_render_passes.Clear();
		Camera::OnPause();
		m_display->OnPause();
	}

	void Graphics::OnResume()
	{
		m_display->OnResume();
	}

	void Graphics::Deinit()
	{
		m_draw_descriptor_set.reset();
		m_draw_descriptor_set_buffer.reset();
		m_blit_render_passes.Clear();
		m_blit_materials.Clear();
		if (m_blit_mesh)
		{
			m_blit_mesh.reset();
		}

		m_display->Deinit();
		m_display.reset();
	}

	Display* Graphics::GetDisplay()
	{
		return m_display.get();
	}

	void Graphics::Render()
	{
		Graphics::draw_call = 0;

		m_display->BeginFrame();

		Camera::RenderAll();

		m_display->EndFrame();

		m_display->SwapBuffers();
	}

	void Graphics::DrawQuad(const Rect* rect, const Ref<Texture>& texture, bool reverse_uv_y)
	{
		Ref<Material> material;
		for (auto& i : m_blit_materials)
		{
			if (i->GetMainTexture() == texture)
			{
				material = i;
				break;
			}
		}

		if (!material)
		{
			material = Material::Create("Blit");
			material->SetMainTexture(texture);
			material->UpdateUniforms(0);

			m_blit_materials.Add(material);
		}

		Graphics::DrawQuad(rect, material, 0, reverse_uv_y);
	}

	void Graphics::DrawQuad(const Rect* rect, const Ref<Material>& material, int pass, bool reverse_uv_y)
	{
		Matrix4x4 world;

		float scale_y = 1;
		if (reverse_uv_y)
		{
			scale_y = -1;
		}

		if (rect == NULL)
		{
			world = Matrix4x4::Scaling(Vector3(1, scale_y, 1));
		}
		else
		{
			Rect quad = Rect(rect->x * 2 - 1, rect->y * 2 - 1, rect->width * 2, rect->height * 2);
			world = Matrix4x4::TRS(
				Vector3(quad.x + quad.width / 2, quad.y + quad.height / 2, 0),
				Quaternion::Identity(),
				Vector3(quad.width / 2, quad.height / 2 * scale_y, 1));
		}

		if (!m_blit_mesh)
		{
			auto mesh = Mesh::Create();
			mesh->vertices.Add(Vector3(-1, -1, 0));
			mesh->vertices.Add(Vector3(1, -1, 0));
			mesh->vertices.Add(Vector3(1, 1, 0));
			mesh->vertices.Add(Vector3(-1, 1, 0));
			mesh->uv.Add(Vector2(0, 1));
			mesh->uv.Add(Vector2(1, 1));
			mesh->uv.Add(Vector2(1, 0));
			mesh->uv.Add(Vector2(0, 0));
			unsigned short triangles[] = {
				0, 1, 2, 0, 2, 3
			};
			mesh->triangles.AddRange(triangles, 6);
			mesh->Update();

			m_blit_mesh = mesh;
		}

		Graphics::DrawMesh(m_blit_mesh, world, material, pass);
	}

	void Graphics::DrawMesh(const Ref<Mesh>& mesh, const Matrix4x4& matrix, const Ref<Material>& material, int pass)
	{
		auto render_pass = RenderPass::GetRenderPassBinding();
		if (render_pass == NULL)
		{
			Camera::Current()->BeginRenderPass(true);
		}

		auto vp = Camera::Current()->GetProjectionMatrix() * Camera::Current()->GetViewMatrix();
		material->SetMatrix("_ViewProjection", vp);

		auto shader = material->GetShader();
		shader->UpdateRendererDescriptorSet(m_draw_descriptor_set, m_draw_descriptor_set_buffer, &matrix, sizeof(Matrix4x4), -1);

		int pass_begin = 0;
		int pass_end = 0;
		if (pass < 0)
		{
			pass_begin = 0;
			pass_end = shader->GetPassCount();
		}
		else
		{
			pass_begin = pass;
			pass_end = pass_begin + 1;
		}

		for (int i = 0; i < mesh->GetSubmeshCount(); i++)
		{
			for (int j = pass_begin; j < pass_end; j++)
			{
				shader->PreparePass(j);
				material->UpdateUniforms(j);

				shader->BeginPass(j);
				shader->BindSharedMaterial(j, material);
				shader->BindMaterial(j, material, m_draw_descriptor_set);
				shader->BindRendererDescriptorSet(j, m_draw_descriptor_set_buffer, -1);

				auto index_type = IndexType::UnsignedShort;
				int index_start;
				int index_count;
				mesh->GetIndexRange(i, index_start, index_count);

				GetDisplay()->BindVertexArray();
				GetDisplay()->BindVertexBuffer(mesh->GetVertexBuffer().get());
				GetDisplay()->BindIndexBuffer(mesh->GetIndexBuffer().get(), index_type);
				GetDisplay()->BindVertexAttribArray(shader, j);
				GetDisplay()->DrawIndexed(index_start, index_count, index_type);
				GetDisplay()->DisableVertexArray(shader, j);

				shader->EndPass(j);
			}
		}

		if (render_pass == NULL)
		{
			Camera::Current()->EndRenderPass(true);
		}
	}

	void Graphics::Blit(const Ref<RenderTexture>& src, const Ref<RenderTexture>& dest, const Ref<Material>& material, int pass, const Rect* rect)
	{
		Ref<RenderPass> render_pass;

		for (auto& i : m_blit_render_passes)
		{
			if (i->HasFrameBuffer())
			{
				auto frame_buffer = i->GetFrameBuffer();
				if (frame_buffer.color_texture == dest)
				{
					render_pass = i;
					break;
				}
			}
			else
			{
				if (!dest)
				{
					render_pass = i;
					break;
				}
			}
		}

		if (!render_pass)
		{
			render_pass = RenderPass::Create(dest, Ref<RenderTexture>(), CameraClearFlags::Invalidate, false, Rect(0, 0, 1, 1));
			m_blit_render_passes.Add(render_pass);
		}

		GetDisplay()->WaitQueueIdle();
		render_pass->Begin(Color(0, 0, 0, 1));

#if VR_GLES
		bool reverse_uv_y = true;
#else
		bool reverse_uv_y = false;
#endif

		if (material)
		{
			if (!material->HasMainTexture() || material->GetMainTexture() != RefCast<Texture>(src))
			{
				material->SetMainTexture(RefCast<Texture>(src));

				if (pass < 0)
				{
					int pass_count = material->GetShader()->GetPassCount();
					for (int i = 0; i < pass_count; i++)
					{
						material->UpdateUniforms(i);
					}
				}
				else
				{
					material->UpdateUniforms(pass);
				}
			}

			if (pass < 0)
			{
				int pass_count = material->GetShader()->GetPassCount();
				for (int i = 0; i < pass_count; i++)
				{
					Graphics::DrawQuad(rect, material, i, reverse_uv_y);
				}
			}
			else
			{
				Graphics::DrawQuad(rect, material, pass, reverse_uv_y);
			}
		}
		else
		{
			Graphics::DrawQuad(rect, RefCast<Texture>(src), reverse_uv_y);
		}

		render_pass->End();
		GetDisplay()->SubmitQueue(render_pass->GetCommandBuffer());
	}
}
