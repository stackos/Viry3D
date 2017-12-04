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

#include "Renderer.h"
#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"
#include "graphics/LightmapSettings.h"
#include "graphics/Light.h"
#include "graphics/RenderPass.h"
#include "graphics/RenderQueue.h"
#include "ui/UICanvasRenderer.h"
#include "io/MemoryStream.h"
#include "time/Time.h"
#include "MeshRenderer.h"
#include "GameObject.h"
#include "World.h"
#include "Profiler.h"
#include "Debug.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Renderer);

	List<Renderer*> Renderer::m_renderers;
	Map<Camera*, Renderer::Passes> Renderer::m_passes;
	bool Renderer::m_renderers_dirty = true;
	Mutex Renderer::m_mutex;
	bool Renderer::m_passes_dirty = true;
	Ref<VertexBuffer> Renderer::m_static_vertex_buffer;
	Ref<IndexBuffer> Renderer::m_static_index_buffer;
	bool Renderer::m_static_buffers_binding = false;
	int Renderer::m_batching_start = -1;
	int Renderer::m_batching_count = -1;

	void Renderer::Init()
	{
	}

	void Renderer::Deinit()
	{
		m_renderers.Clear();
		m_passes.Clear();
		m_renderers_dirty = true;
		m_passes_dirty = true;
		m_static_vertex_buffer.reset();
		m_static_index_buffer.reset();
		m_static_buffers_binding = false;
		m_batching_start = -1;
		m_batching_count = -1;
	}

	void Renderer::OnResize(int width, int height)
	{
		Shader::ClearAllPipelines();
	}

	void Renderer::OnPause()
	{
		Shader::ClearAllPipelines();
	}

	void Renderer::PreRenderByMaterial(int material_index)
	{
		auto vp = Camera::Current()->GetProjectionMatrix() * Camera::Current()->GetViewMatrix();
		auto& mat = this->GetSharedMaterials()[material_index];
		mat->SetMatrix("_ViewProjection", vp);
		mat->SetVector("_WorldSpaceCameraPos", Camera::Current()->GetTransform()->GetPosition());
		mat->SetVector("_Time", Vector4(Time::GetTime()));

		if (!Light::main.expired())
		{
			auto light = Light::main.lock();
			mat->SetVector("_WorldSpaceLightPos", -light->GetTransform()->GetForward());
			mat->SetColor("_LightColor", light->color * light->intensity);
		}
	}

	void Renderer::PreRenderByRenderer(int material_index)
	{
		struct UniformBufferObject
		{
			Matrix4x4 world_matrix;
			Vector4 lightmap_sacle_offset;
		};
		UniformBufferObject buffer;
		int size;

		bool static_batch = this->m_batch_indices.Size() > 0;
		if (static_batch)
		{
			buffer.world_matrix = Matrix4x4::Identity();
			buffer.lightmap_sacle_offset = Vector4(1, 1, 0, 0);
			size = sizeof(Matrix4x4) + sizeof(Vector4);
		}
		else
		{
			buffer.world_matrix = this->GetWorldMatrix();
			if (m_lightmap_index >= 0)
			{
				buffer.lightmap_sacle_offset = this->GetLightmapScaleOffset();
				size = sizeof(Matrix4x4) + sizeof(Vector4);
			}
			else
			{
				size = sizeof(Matrix4x4);
			}
		}

		auto shader = this->GetSharedMaterials()[material_index]->GetShader();
		if(Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}
		shader->UpdateRendererDescriptorSet(m_descriptor_set, m_descriptor_set_buffer, &buffer, size, m_lightmap_index);
	}

	Matrix4x4 Renderer::GetWorldMatrix()
	{
		return GetTransform()->GetLocalToWorldMatrix();
	}

	void Renderer::Render(int material_index, int pass_index)
	{
		auto& mat = this->GetSharedMaterials()[material_index];
		auto shader = mat->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}
		bool static_batch = this->m_batch_indices.Size() > 0;
		bool batching = m_batching_start >= 0 && m_batching_count > 0;

		auto index_type = GetIndexType();
		if (static_batch)
		{
			index_type = IndexType::UnsignedInt;
		}

		if (this->GetVertexBuffer() || static_batch)
		{
			if (!static_batch)
			{
				m_static_buffers_binding = false;
				Graphics::GetDisplay()->BindVertexArray();
				Graphics::GetDisplay()->BindVertexBuffer(this->GetVertexBuffer());
				Graphics::GetDisplay()->BindIndexBuffer(this->GetIndexBuffer(), index_type);
			}
			else
			{
				if (!m_static_buffers_binding)
				{
					m_static_buffers_binding = true;
					BindStaticBuffers();
				}
			}

			if (!static_batch || !batching)
			{
				Graphics::GetDisplay()->BindVertexAttribArray(shader, pass_index);
			}

			int start, count;
			if (static_batch)
			{
				start = m_batch_indices[material_index].index_start;
				count = m_batch_indices[material_index].index_count;
			}
			else
			{
				this->GetIndexRange(material_index, start, count);
			}

			if (static_batch)
			{
				if (!batching)
				{
					// 第一批
					m_batching_start = start;
					m_batching_count = count;
				}
			}
			else
			{
				Graphics::GetDisplay()->DrawIndexed(start, count, index_type);
				Graphics::GetDisplay()->DisableVertexArray(shader, pass_index);
			}
		}
	}

	void Renderer::CommitPass(List<MaterialPass>& pass)
	{
		auto& first = pass.First();
		auto shader = first.renderer->GetSharedMaterials()[first.material_index]->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		if (first.shader_pass_count == 1)
		{
			shader->BeginPass(0);

			int old_id = -1;
			int old_lightmap_index = -1;
			for (auto& i : pass)
			{
				auto& mat = i.renderer->GetSharedMaterials()[i.material_index];
				bool bind_shared_mat = false;
				bool bind_lightmap = false;
				bool static_batch = i.renderer->m_batch_indices.Size() > 0;
				bool batching = m_batching_start >= 0 && m_batching_count > 0;
				bool batching_break = false;

				int mat_id = mat->GetId();
				if (old_id == -1 || old_id != mat_id)
				{
					bind_shared_mat = true;
					bind_lightmap = true;
				}
				old_id = mat_id;

				if (old_lightmap_index == -1 || old_lightmap_index != i.renderer->m_lightmap_index)
				{
					bind_lightmap = true;
				}
				old_lightmap_index = i.renderer->m_lightmap_index;

				if (batching)
				{
					if (bind_shared_mat || bind_lightmap || !static_batch)
					{
						batching_break = true;
					}
				}

				if (static_batch)
				{
					int start, count;
					start = i.renderer->m_batch_indices[i.material_index].index_start;
					count = i.renderer->m_batch_indices[i.material_index].index_count;

					if (batching)
					{
						if (m_batching_start + m_batching_count == start)
						{
							// 连续索引，追加到合并批次
							m_batching_count += count;
						}
						else
						{
							batching_break = true;
						}
					}
				}

				if (batching_break)
				{
					// 提交合并绘制
					Graphics::GetDisplay()->DrawIndexed(m_batching_start, m_batching_count, IndexType::UnsignedInt);
					Graphics::GetDisplay()->DisableVertexArray(shader, 0);

					m_batching_start = -1;
					m_batching_count = -1;
					batching = false;
				}

				if (bind_shared_mat)
				{
					shader->BindSharedMaterial(0, mat);
				}

				// 非静态或第一批
				if (!static_batch || !batching)
				{
					shader->BindMaterial(0, mat, i.renderer->m_descriptor_set);
					shader->BindRendererDescriptorSet(0, i.renderer->m_descriptor_set_buffer, i.renderer->m_lightmap_index);
				}

				i.renderer->Render(i.material_index, 0);
			}

			// pass完成，提交剩余批次
			if (m_batching_start >= 0 && m_batching_count > 0)
			{
				Graphics::GetDisplay()->DrawIndexed(m_batching_start, m_batching_count, IndexType::UnsignedInt);
				Graphics::GetDisplay()->DisableVertexArray(shader, 0);

				m_batching_start = -1;
				m_batching_count = -1;
			}

			shader->EndPass(0);
		}
		else
		{
			assert(pass.Size() == 1);

			auto& i = first;
			for (int j = 0; j < i.shader_pass_count; j++)
			{
				int pass_index = j;
				if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
				{
					pass_index = 0;
				}

				shader->BeginPass(pass_index);

				auto& mat = i.renderer->GetSharedMaterials()[i.material_index];
				shader->BindSharedMaterial(pass_index, mat);
				shader->BindMaterial(pass_index, mat, i.renderer->m_descriptor_set);
				shader->BindRendererDescriptorSet(pass_index, i.renderer->m_descriptor_set_buffer, i.renderer->m_lightmap_index);

				i.renderer->Render(i.material_index, pass_index);

				shader->EndPass(pass_index);
			}
		}
	}

	void Renderer::PreparePass(List<MaterialPass>& pass)
	{
		auto& first = pass.First();
		auto shader = first.renderer->GetSharedMaterials()[first.material_index]->GetShader();
		if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
		{
			shader = Shader::ReplaceToShadowMapShader(shader);
		}

		if (first.shader_pass_count == 1)
		{
			shader->PreparePass(0);

			int old_id = -1;
			for (auto& i : pass)
			{
				auto& mat = i.renderer->GetSharedMaterials()[i.material_index];
				int mat_id = mat->GetId();

				i.renderer->PreRenderByRenderer(i.material_index);

				if (old_id == -1 || old_id != mat_id)
				{
					i.renderer->PreRenderByMaterial(i.material_index);
					mat->UpdateUniforms(0);
				}

				old_id = mat_id;
			}
		}
		else
		{
			auto& mat = first.renderer->GetSharedMaterials()[first.material_index];
			first.renderer->PreRenderByRenderer(first.material_index);
			first.renderer->PreRenderByMaterial(first.material_index);

			for (int i = 0; i < first.shader_pass_count; i++)
			{
				int pass_index = i;
				if (Camera::Current()->GetRenderMode() == CameraRenderMode::ShadowMap)
				{
					pass_index = 0;
				}

				shader->PreparePass(pass_index);
				mat->UpdateUniforms(pass_index);
			}
		}
	}

	bool Renderer::IsRenderersDirty()
	{
		bool dirty;
		m_mutex.lock();
		dirty = m_renderers_dirty;
		m_mutex.unlock();
		return dirty;
	}

	void Renderer::SetRenderersDirty(bool dirty)
	{
		m_mutex.lock();
		m_renderers_dirty = dirty;
		m_mutex.unlock();
	}

	void Renderer::ClearPasses()
	{
		m_passes.Clear();
	}

	void Renderer::SetCullingDirty(Camera* cam)
	{
		if (m_passes.Contains(cam))
		{
			m_passes[cam].culling_dirty = true;
		}
	}

	List<Renderer*>& Renderer::GetRenderers()
	{
		return m_renderers;
	}

	void Renderer::HandleUIEvent()
	{
		List<UICanvasRenderer*> canvas_list;
		for (auto i : m_renderers)
		{
			if (!i->GetGameObject()->IsActiveInHierarchy() ||
				!i->IsEnable())
			{
				continue;
			}

			auto canvas = dynamic_cast<UICanvasRenderer*>(i);
			if (canvas != NULL)
			{
				canvas_list.AddLast(canvas);
			}
		}

		canvas_list.Sort([](UICanvasRenderer* a, UICanvasRenderer* b) {
			return a->GetSortingOrder() < b->GetSortingOrder();
		});

		UICanvasRenderer::HandleUIEvent(canvas_list);
	}

	void Renderer::CheckPasses()
	{
		Vector<Camera*> invalid_cams;
		for (auto& i : m_passes)
		{
			if (!Camera::IsValidCamera(i.first))
			{
				invalid_cams.Add(i.first);
			}
		}
		for (auto i : invalid_cams)
		{
			m_passes.Remove(i);
		}

		auto cam = Camera::Current();
		if (!m_passes.Contains(cam))
		{
			m_passes.Add(cam, Passes());
		}
	}

	void Renderer::CameraCulling()
	{
		auto cam = Camera::Current();

		if (m_passes[cam].culling_dirty)
		{
			m_passes[cam].culling_dirty = false;

			List<Renderer*> renderers;
			auto& culled_renderers = m_passes[cam].culled_renderers;
			bool diff = false;

			// TODO: m_renderers in octree

			for (auto i : m_renderers)
			{
				if (!i->GetGameObject()->IsActiveInHierarchy() ||
					!i->IsEnable() ||
					cam->IsCulling(i->GetGameObject()))
				{
					continue;
				}

				if (cam->IsOrthographic() || cam->IsFrustumCulling() == false)
				{
					renderers.AddLast(i);
				}
				else
				{
					auto& bounds = i->GetBounds();
					auto ret = cam->GetFrustum().ContainsBounds(bounds.Min(), bounds.Max());
					if (ret == ContainsResult::Cross || ret == ContainsResult::In)
					{
						renderers.AddLast(i);
					}
				}
			}

			if (renderers.Size() != culled_renderers.Size())
			{
				diff = true;
			}
			else
			{
				auto ia = renderers.begin();
				auto ib = culled_renderers.begin();

				while (ia != renderers.end() && ib != culled_renderers.end())
				{
					auto a = *ia;
					auto b = *ib;
					if (a != b)
					{
						diff = true;
						break;
					}

					ia++;
					ib++;
				}
			}

			if (diff)
			{
				culled_renderers = renderers;
				m_passes[cam].passes_dirty = true;
			}
		}
	}

	void Renderer::BuildPasses(const List<Renderer*>& renderers, List<List<MaterialPass>>& passes)
	{
		List<Renderer::MaterialPass> mat_passes;

		for (auto i : renderers)
		{
			auto& mats = i->GetSharedMaterials();

			for (int j = 0; j < mats.Size(); j++)
			{
				auto& mat = mats[j];

				if (!mat || !i->IsValidPass(j))
				{
					continue;
				}

				auto shader = mat->GetShader();
				int pass_count = shader->GetPassCount();

				assert(pass_count >= 1);

				MaterialPass pass;
				pass.queue = shader->GetQueue();
				pass.shader_pass_count = pass_count;
				pass.renderer = i;
				pass.material_index = j;
				pass.shader_id = shader->GetId();
				pass.material_id = mat->GetId();

				mat_passes.AddLast(pass);
			}
		}

		mat_passes.Sort(
			[](const MaterialPass& a, const MaterialPass& b)->bool {
			if (dynamic_cast<UICanvasRenderer*>(a.renderer) || dynamic_cast<UICanvasRenderer*>(b.renderer))
			{
				return false;
			}

			if (a.queue == b.queue)
			{
				int static_a = a.renderer->GetGameObject()->IsStatic() ? 0 : 1;
				int static_b = b.renderer->GetGameObject()->IsStatic() ? 0 : 1;

				if (static_a == static_b)
				{
					if (a.shader_pass_count == 1 && b.shader_pass_count == 1)
					{
						if (a.shader_id == b.shader_id)
						{
							if (a.material_id == b.material_id)
							{
								if (a.renderer->m_lightmap_index == b.renderer->m_lightmap_index)
								{
									if (a.renderer->GetId() == b.renderer->GetId())
									{
										return a.material_index < b.material_index;
									}
									else
									{
										return a.renderer->GetId() < b.renderer->GetId();
									}
								}
								else
								{
									return a.renderer->m_lightmap_index < b.renderer->m_lightmap_index;
								}
							}
							else
							{
								return a.material_id < b.material_id;
							}
						}
						else
						{
							return a.shader_id < b.shader_id;
						}
					}
					else
					{
						return a.shader_pass_count < b.shader_pass_count;
					}
				}
				else
				{
					return static_a < static_b;
				}
			}
			else
			{
				return a.queue < b.queue;
			}
		});

		passes.Clear();

		List<MaterialPass> pass;
		for (auto& i : mat_passes)
		{
			if (pass.Empty())
			{
				pass.AddLast(i);
			}
			else
			{
				const auto& last = pass.Last();
				if (dynamic_cast<UICanvasRenderer*>(i.renderer) == NULL &&
					i.queue == last.queue &&
					i.shader_pass_count == 1 && last.shader_pass_count == 1 &&
					i.shader_id == last.shader_id)
				{
					pass.AddLast(i);
				}
				else
				{
					passes.AddLast(pass);

					pass.Clear();
					pass.AddLast(i);
				}
			}
		}

		if (!pass.Empty())
		{
			passes.AddLast(pass);
		}
	}

	void Renderer::BuildPasses()
	{
		auto cam = Camera::Current();

		if (m_passes[cam].passes_dirty)
		{
			m_passes[cam].passes_dirty = false;

			BuildPasses(m_passes[cam].culled_renderers, m_passes[cam].list);
		}
	}

	void Renderer::PrepareAllPass()
	{
		CheckPasses();
		CameraCulling();
		BuildPasses();

		auto& passes = m_passes[Camera::Current()].list;
		for (auto& i : passes)
		{
			Renderer::PreparePass(i);
		}
	}

	void Renderer::BindStaticBuffers()
	{
		if (m_static_vertex_buffer && m_static_index_buffer)
		{
			Graphics::GetDisplay()->BindVertexBuffer(m_static_vertex_buffer.get());
			Graphics::GetDisplay()->BindIndexBuffer(m_static_index_buffer.get(), IndexType::UnsignedInt);
		}
	}

	void Renderer::RenderAllPass()
	{
		BindStaticBuffers();
		m_static_buffers_binding = true;
		m_batching_start = -1;
		m_batching_count = -1;

		auto cam = Camera::Current();
		auto& passes = m_passes[cam].list;
		List<List<MaterialPass>> passes_transparent;
		List<List<MaterialPass>> passes_ui;

		for (auto& i : passes)
		{
			if (i.First().queue < (int) RenderQueue::Transparent)
			{
				Renderer::CommitPass(i);
			}
			else
			{
				auto ui = dynamic_cast<UICanvasRenderer*>(i.First().renderer);
				if (ui)
				{
					passes_ui.AddLast(i);
				}
				else
				{
					passes_transparent.AddLast(i);
				}
			}
		}

		for (auto& i : passes_transparent)
		{
			Renderer::CommitPass(i);
		}

		passes_ui.Sort([](const List<MaterialPass>& a, const List<MaterialPass>& b) {
			if (a.First().queue == b.First().queue)
			{
				return a.First().renderer->GetSortingOrder() < b.First().renderer->GetSortingOrder();
			}
			else
			{
				return a.First().queue < b.First().queue;
			}
		});

		for (auto& i : passes_ui)
		{
			Renderer::CommitPass(i);
		}
	}

	Renderer::Renderer():
		m_sorting_order(0),
		m_lightmap_index(-1),
		m_lightmap_scale_offset(),
		m_bounds(Vector3::One() * Mathf::MinFloatValue, Vector3::One() * Mathf::MaxFloatValue)
	{
	}

	Renderer::~Renderer()
	{
		SetRenderersDirty(true);
	}

	void Renderer::Start()
	{
		SetRenderersDirty(true);
	}

	void Renderer::OnEnable()
	{
		SetRenderersDirty(true);
	}

	void Renderer::OnDisable()
	{
		SetRenderersDirty(true);
	}

	Ref<Material> Renderer::GetSharedMaterial() const
	{
		Ref<Material> mat;

		auto& mats = this->GetSharedMaterials();
		if (!mats.Empty())
		{
			mat = mats[0];
		}

		return mat;
	}

	void Renderer::SetSharedMaterial(const Ref<Material>& mat)
	{
		auto mats = Vector<Ref<Material>>();
		mats.Add(mat);

		this->SetSharedMaterials(mats);
	}

	void Renderer::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<Renderer>(source);
		this->SetSharedMaterials(src->GetSharedMaterials());
	}

	void Renderer::BuildStaticBatch(const Ref<GameObject>& obj)
	{
		List<Renderer*> renderers;
		List<List<MaterialPass>> passes;
		List<MaterialPass> passes_all;

		auto rs = obj->GetComponentsInChildren<MeshRenderer>();
		for (const auto& r : rs)
		{
			renderers.AddLast(r.get());
		}

		BuildPasses(renderers, passes);

		int vertex_count = 0;
		int index_count = 0;

		for (const auto& i : passes)
		{
			auto& first = i.First();

			if (first.shader_pass_count == 1)
			{
				for (auto& j : i)
				{
					passes_all.AddLast(j);
				}
			}
			else
			{
				passes_all.AddLast(first);
			}
		}

		for (const auto& i : passes_all)
		{
			auto r = (MeshRenderer*) i.renderer;
			auto& mesh = r->GetSharedMesh();
			int submesh = i.material_index;

			if (submesh >= r->m_batch_indices.Size())
			{
				r->m_batch_indices.Resize(submesh + 1);
			}

			int start;
			int count;
			mesh->GetIndexRange(submesh, start, count);

			r->m_batch_indices[submesh].index_start = index_count;
			r->m_batch_indices[submesh].index_count = count;

			vertex_count += mesh->vertices.Size();
			index_count += count;
		}

		m_static_vertex_buffer = VertexBuffer::Create(vertex_count * VERTEX_STRIDE, false);
		m_static_vertex_buffer->Fill(NULL,
			[&](void* param, const ByteBuffer& buffer) {
			MemoryStream ms(buffer);

			for (const auto& i : passes_all)
			{
				auto r = (MeshRenderer*) i.renderer;
				auto& mesh = r->GetSharedMesh();
				auto& mat = r->GetTransform()->GetLocalToWorldMatrix();

				for (int i = 0; i < mesh->vertices.Size(); i++)
				{
					auto pos_world = mat.MultiplyPoint3x4(mesh->vertices[i]);
					ms.Write<Vector3>(pos_world);

					if (mesh->colors.Empty())
					{
						ms.Write<Color>(Color(1, 1, 1, 1));
					}
					else
					{
						ms.Write<Color>(mesh->colors[i]);
					}

					if (mesh->uv.Empty())
					{
						ms.Write<Vector2>(Vector2(0, 0));
					}
					else
					{
						ms.Write<Vector2>(mesh->uv[i]);
					}

					if (mesh->uv2.Empty())
					{
						ms.Write<Vector2>(Vector2(0, 0));
					}
					else
					{
						auto& scale_offset = r->GetLightmapScaleOffset();
						auto uv2 = mesh->uv2[i];
						float x = uv2.x;
						float y = 1.0f - uv2.y;
						x = x * scale_offset.x + scale_offset.z;
						y = y * scale_offset.y + scale_offset.w;
						y = 1.0f - y;
						ms.Write<Vector2>(Vector2(x, y));
					}

					if (mesh->normals.Empty())
					{
						ms.Write<Vector3>(Vector3(0, 0, 0));
					}
					else
					{
						auto normal_world = mat.MultiplyDirection(mesh->normals[i]);
						ms.Write<Vector3>(normal_world);
					}

					if (mesh->tangents.Empty())
					{
						ms.Write<Vector4>(Vector4(0, 0, 0, 0));
					}
					else
					{
						auto tangent = mesh->tangents[i];
						auto tangent_xyz = Vector3(tangent.x, tangent.y, tangent.z);
						auto tangent_xyz_world = mat.MultiplyDirection(tangent_xyz);
						auto tangent_world = Vector4(tangent_xyz_world.x, tangent_xyz_world.y, tangent_xyz_world.z, tangent.w);
						ms.Write<Vector4>(tangent_world);
					}

					if (mesh->bone_weights.Empty())
					{
						ms.Write<Vector4>(Vector4(0, 0, 0, 0));
					}
					else
					{
						ms.Write<Vector4>(mesh->bone_weights[i]);
					}

					if (mesh->bone_indices.Empty())
					{
						ms.Write<Vector4>(Vector4(0, 0, 0, 0));
					}
					else
					{
						ms.Write<Vector4>(mesh->bone_indices[i]);
					}
				}
			}

			ms.Close();
		});

		m_static_index_buffer = IndexBuffer::Create(index_count * sizeof(unsigned int), false);
		m_static_index_buffer->Fill(NULL,
			[&](void* param, const ByteBuffer& buffer) {
			MemoryStream ms(buffer);

			int mesh_index_offset = 0;

			for (const auto& i : passes_all)
			{
				auto r = (MeshRenderer*) i.renderer;
				auto& mesh = r->GetSharedMesh();
				int submesh = i.material_index;

				int start;
				int count;
				mesh->GetIndexRange(submesh, start, count);

				for (int j = 0; j < count; j++)
				{
					unsigned int index = mesh->triangles[start + j] + mesh_index_offset;
					ms.Write<unsigned int>(index);
				}

				mesh_index_offset += mesh->vertices.Size();
			}

			ms.Close();
		});
	}
}
