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

#include "ParticleSystem.h"
#include "ParticleSystemRenderer.h"
#include "GameObject.h"
#include "memory/Memory.h"
#include "io/MemoryStream.h"
#include "time/Time.h"
#include "math/Mathf.h"
#include "graphics/VertexAttribute.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(ParticleSystem);

	ParticleSystem::ParticleSystem():
		m_time_start(0),
		m_start_delay(0),
		m_time(0),
		m_time_emit(-1)
	{
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	void ParticleSystem::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<ParticleSystem>(source);
		this->main = src->main;
		this->emission = src->emission;
		this->shape = src->shape;
		this->velocity_over_lifetime = src->velocity_over_lifetime;
		this->limit_velocity_over_lifetime = src->limit_velocity_over_lifetime;
		this->inherit_velocity = src->inherit_velocity;
		this->force_over_lifetime = src->force_over_lifetime;
		this->color_over_lifetime = src->color_over_lifetime;
		this->color_by_speed = src->color_by_speed;
		this->size_over_lifetime = src->size_over_lifetime;
		this->size_by_speed = src->size_by_speed;
		this->rotation_over_lifetime = src->rotation_over_lifetime;
		this->rotation_by_speed = src->rotation_by_speed;
		this->external_forces = src->external_forces;
		this->texture_sheet_animation = src->texture_sheet_animation;
	}

	void ParticleSystem::Start()
	{
		m_start_delay = main.start_delay.Evaluate(0);
		m_time_start = Time::GetTime() + m_start_delay;
		m_renderer = this->GetGameObject()->GetComponent<ParticleSystemRenderer>();
	}

	void ParticleSystem::Update()
	{
		if (!m_renderer || !emission.enabled)
		{
			return;
		}

		UpdateEmission();
		UpdateParticles();
	}

	int ParticleSystem::GetParticleCount() const
	{
		return m_partices.Size();
	}

	void ParticleSystem::Emit(Particle& p)
	{
		m_partices.AddLast(p);
		m_time_emit = Time::GetTime();
	}

	void ParticleSystem::UpdateEmission()
	{
		if (!emission.enabled)
		{
			return;
		}

		if (CheckTime())
		{
			float now = Time::GetTime();
			float t = m_time;
			float rate = emission.rate_over_time.Evaluate(t);
			int emit_count = 0;

			for (auto& burst : emission.bursts)
			{
				if (m_time >= burst.time)
				{
					if (burst.cycle_count <= 0 || burst.cycle_count > burst.emit_count)
					{
						if (burst.emit_time < 0 || now - burst.emit_time >= burst.repeat_interval)
						{
							burst.emit_time = now;
							burst.emit_count++;
							emit_count += Mathf::RandomRange(burst.min_count, burst.max_count + 1);
						}
					}
				}
			}

			if (rate > 0 && (m_time_emit < 0 || (now - m_time_emit) * main.simulation_speed >= 1.0f / rate))
			{
				emit_count += 1;
			}

			if (emit_count > 0)
			{
				int can_emit_count = main.max_particles - GetParticleCount();
				emit_count = Mathf::Min(emit_count, can_emit_count);

				for (int i = 0; i < emit_count; i++)
				{
					float start_speed = main.start_speed.Evaluate(t);
					Color start_color = main.start_color.Evaluate(t);
					float start_lifetime = main.start_lifetime.Evaluate(t);
					float remaining_lifetime = start_lifetime;
					Vector3 start_size(0, 0, 0);
					Vector3 position(0, 0, 0);
					Vector3 rotation(0, 0, 0);
					Vector3 velocity(0, 0, 1);

					if (main.start_size_3d)
					{
						start_size.x = main.start_size_x.Evaluate(t);
						start_size.y = main.start_size_y.Evaluate(t);
						start_size.z = main.start_size_z.Evaluate(t);
					}
					else
					{
						float size = main.start_size.Evaluate(t);
						start_size = Vector3(size, size, size);
					}

					if (main.start_rotation_3d)
					{
						rotation.x = main.start_rotation_x.Evaluate(t);
						rotation.y = main.start_rotation_y.Evaluate(t);
						rotation.z = main.start_rotation_z.Evaluate(t);
					}
					else
					{
						rotation.x = 0;
						rotation.y = 0;
						rotation.z = main.start_rotation.Evaluate(t);
					}

					if (shape.enabled)
					{
						switch (shape.shape_type)
						{
							case ParticleSystemShapeType::Sphere:
							case ParticleSystemShapeType::SphereShell:
								EmitShapeSphere(position, velocity, false);
								break;
							case ParticleSystemShapeType::Hemisphere:
							case ParticleSystemShapeType::HemisphereShell:
								EmitShapeSphere(position, velocity, true);
								break;
							case ParticleSystemShapeType::Cone:
							case ParticleSystemShapeType::ConeShell:
							case ParticleSystemShapeType::ConeVolume:
							case ParticleSystemShapeType::ConeVolumeShell:
								EmitShapeCone(position, velocity);
								break;
							case ParticleSystemShapeType::Box:
							case ParticleSystemShapeType::BoxShell:
							case ParticleSystemShapeType::BoxEdge:
								EmitShapeBox(position, velocity);
								break;
							case ParticleSystemShapeType::Circle:
							case ParticleSystemShapeType::CircleEdge:
								EmitShapeCircle(position, velocity);
								break;
							case ParticleSystemShapeType::SingleSidedEdge:
								EmitShapeEdge(position, velocity);
								break;
							default:
								Log("not implement particle emit shape: %d", shape.shape_type);
								break;
						}
					}

					Particle p;
					p.emit_time = Time::GetTime();
					p.start_lifetime = start_lifetime;
					p.remaining_lifetime = remaining_lifetime;
					p.start_size = start_size;
					p.angular_velocity = Vector3::Zero();
					p.rotation = rotation;
					p.start_color = start_color;
					p.color = start_color;
					if (main.simulation_space == ParticleSystemSimulationSpace::World)
					{
						auto& mat = this->GetTransform()->GetLocalToWorldMatrix();
						p.velocity = mat.MultiplyDirection(velocity * start_speed);
						p.position = mat.MultiplyPoint3x4(position);
					}
					else
					{
						p.velocity = velocity * start_speed;
						p.position = position;
					}
					p.start_velocity = p.velocity;
					p.force_velocity = Vector3::Zero();

					// init random values
					if (rotation_over_lifetime.enabled && rotation_over_lifetime.z.mode == ParticleSystemCurveMode::TwoConstants)
					{
						if (rotation_over_lifetime.separate_axes)
						{
							p.rotation_over_lifetime_random.x = rotation_over_lifetime.x.Evaluate(0);
							p.rotation_over_lifetime_random.y = rotation_over_lifetime.y.Evaluate(0);
							p.rotation_over_lifetime_random.z = rotation_over_lifetime.z.Evaluate(0);
						}
						else
						{
							p.rotation_over_lifetime_random.x = 0;
							p.rotation_over_lifetime_random.y = 0;
							p.rotation_over_lifetime_random.z = rotation_over_lifetime.z.Evaluate(0);
						}

					}

					if (texture_sheet_animation.enabled)
					{
						if (texture_sheet_animation.animation == ParticleSystemAnimationType::SingleRow && texture_sheet_animation.use_random_row)
						{
							p.texture_sheet_animation_row = Mathf::RandomRange(0, texture_sheet_animation.num_tiles_y);
						}

						if (texture_sheet_animation.animation == ParticleSystemAnimationType::SingleRow)
						{
							p.texture_sheet_animation_start_frame = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.start_frame.Evaluate(0));
						}
						else
						{
							p.texture_sheet_animation_start_frame = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.num_tiles_y * texture_sheet_animation.start_frame.Evaluate(0));
						}

						if (texture_sheet_animation.frame_over_time.mode == ParticleSystemCurveMode::TwoConstants)
						{
							if (texture_sheet_animation.animation == ParticleSystemAnimationType::SingleRow)
							{
								p.texture_sheet_animation_frame = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.frame_over_time.Evaluate(0));
							}
							else
							{
								p.texture_sheet_animation_frame = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.num_tiles_y * texture_sheet_animation.frame_over_time.Evaluate(0));
							}
						}
					}

					Emit(p);
				}
			}
		}
	}

	bool ParticleSystem::CheckTime()
	{
		float now = Time::GetTime();
		float duration = main.duration;
		if (now >= m_time_start && duration > 0)
		{
			float play_time = (now - m_time_start) * main.simulation_speed;
			if (play_time < duration)
			{
				m_time = play_time;
				return true;
			}
			else if (main.loop)
			{
				m_time = 0;
				m_time_start = Time::GetTime();
				emission.ResetBurstState();
				return true;
			}
		}

		return false;
	}

	void ParticleSystem::UpdateParticles()
	{
		for (auto i = m_partices.Begin(); i != m_partices.End(); )
		{
			auto& p = i->value;

			if (p.remaining_lifetime > 0)
			{
				UpdateParticleVelocity(p);
				UpdateParticleAngularVelocity(p);
				UpdateParticleColor(p);
				UpdateParticleSize(p);
				UpdateParticleUV(p);
				UpdateParticlePosition(p);
				UpdateParticleRotation(p);
				UpdateParticleLifetime(p);
			}

			if (p.remaining_lifetime <= 0)
			{
				i = m_partices.Remove(i);
				continue;
			}

			i = i->next;
		}
	}

	void ParticleSystem::FillVertexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto ps = (ParticleSystem*) param;
		auto vs = (Vertex*) buffer.Bytes();

		auto camera_to_world = Camera::Current()->GetTransform()->GetLocalToWorldMatrix();
		auto world_to_camera = Camera::Current()->GetTransform()->GetWorldToLocalMatrix();

		int index = 0;
		for (auto i = ps->m_partices.Begin(); i != ps->m_partices.End(); i = i->next, index++)
		{
			const auto& p = i->value;

			Vector3 pos_world;
			Vector3 velocity_world;

			if (ps->main.simulation_space == ParticleSystemSimulationSpace::World)
			{
				pos_world = p.position;
				velocity_world = p.velocity;
			}
			else
			{
				pos_world = ps->GetTransform()->GetLocalToWorldMatrix().MultiplyPoint3x4(p.position);
				velocity_world = ps->GetTransform()->GetLocalToWorldMatrix().MultiplyDirection(p.velocity);
			}

			auto render_mode = ps->m_renderer->render_mode;
			if (render_mode == ParticleSystemRenderMode::Stretch && Mathf::FloatEqual(p.velocity.SqrMagnitude(), 0))
			{
				render_mode = ParticleSystemRenderMode::Billboard;
			}

			if (render_mode == ParticleSystemRenderMode::Billboard)
			{
				auto& v0 = vs[index * 4 + 0];
				auto& v1 = vs[index * 4 + 1];
				auto& v2 = vs[index * 4 + 2];
				auto& v3 = vs[index * 4 + 3];

				auto rot = Quaternion::Euler(p.rotation * Mathf::Rad2Deg);
				Vector3 pos_view = world_to_camera.MultiplyPoint3x4(pos_world);
				v0.vertex = pos_view + rot * Vector3(-p.size.x * 0.5f, p.size.y * 0.5f, 0.0f);
				v1.vertex = pos_view + rot * Vector3(-p.size.x * 0.5f, -p.size.y * 0.5f, 0.0f);
				v2.vertex = pos_view + rot * Vector3(p.size.x * 0.5f, -p.size.y * 0.5f, 0.0f);
				v3.vertex = pos_view + rot * Vector3(p.size.x * 0.5f, p.size.y * 0.5f, 0.0f);
				v0.vertex = camera_to_world.MultiplyPoint3x4(v0.vertex);
				v1.vertex = camera_to_world.MultiplyPoint3x4(v1.vertex);
				v2.vertex = camera_to_world.MultiplyPoint3x4(v2.vertex);
				v3.vertex = camera_to_world.MultiplyPoint3x4(v3.vertex);
			}
			else if (render_mode == ParticleSystemRenderMode::Stretch)
			{
				auto& v0 = vs[index * 4 + 0];
				auto& v1 = vs[index * 4 + 1];
				auto& v2 = vs[index * 4 + 2];
				auto& v3 = vs[index * 4 + 3];

				Vector3 forward = velocity_world;
				Vector3 right = velocity_world * Camera::Current()->GetTransform()->GetForward();
				Quaternion rot;
				if (Mathf::FloatEqual(right.SqrMagnitude(), 0))
				{
					rot = Quaternion::FromToRotation(Vector3(0, 0, 1), forward);
				}
				else
				{
					Vector3 up = forward * right;
					rot = Quaternion::LookRotation(forward, up);
				}
				float length = p.size.y * ps->m_renderer->length_scale + velocity_world.Magnitude() * ps->m_renderer->velocity_scale;
				v0.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, 0.0f, length * 0.5f);
				v1.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, 0.0f, -length * 0.5f);
				v2.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, 0.0f, -length * 0.5f);
				v3.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, 0.0f, length * 0.5f);
			}
			else if (render_mode == ParticleSystemRenderMode::HorizontalBillboard)
			{
				auto& v0 = vs[index * 4 + 0];
				auto& v1 = vs[index * 4 + 1];
				auto& v2 = vs[index * 4 + 2];
				auto& v3 = vs[index * 4 + 3];
				
				auto rot = Quaternion::Euler(Vector3(0, 0, p.rotation.z) * Mathf::Rad2Deg);
				v0.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, 0, p.size.y * 0.5f);
				v1.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, 0, -p.size.y * 0.5f);
				v2.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, 0, -p.size.y * 0.5f);
				v3.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, 0, p.size.y * 0.5f);
			}
			else if (render_mode == ParticleSystemRenderMode::VerticalBillboard)
			{
				auto& v0 = vs[index * 4 + 0];
				auto& v1 = vs[index * 4 + 1];
				auto& v2 = vs[index * 4 + 2];
				auto& v3 = vs[index * 4 + 3];

				auto cam_rot = Camera::Current()->GetTransform()->GetRotation().ToEulerAngles();
				auto rot = Quaternion::Euler(Vector3(0, cam_rot.y, p.rotation.z * Mathf::Rad2Deg));
				v0.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, p.size.y * 0.5f, 0);
				v1.vertex = pos_world + rot * Vector3(-p.size.x * 0.5f, -p.size.y * 0.5f, 0);
				v2.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, -p.size.y * 0.5f, 0);
				v3.vertex = pos_world + rot * Vector3(p.size.x * 0.5f, p.size.y * 0.5f, 0);
			}
			else if (render_mode == ParticleSystemRenderMode::Mesh)
			{
				auto rot = Quaternion::Euler(p.rotation * Mathf::Rad2Deg);

				const auto& mesh = ps->m_renderer->mesh;
				int vertex_count = mesh->vertices.Size();
				for (int j = 0; j < vertex_count; j++)
				{
					auto& v = vs[index * vertex_count + j];
					auto local_to_world = Matrix4x4::TRS(Vector3::Zero(), ps->GetTransform()->GetRotation(), ps->GetTransform()->GetScale()) * Matrix4x4::TRS(Vector3::Zero(), rot, p.size);
					v.vertex = pos_world + local_to_world.MultiplyPoint3x4(mesh->vertices[j]);
					v.uv = mesh->uv[j];
					if (mesh->colors.Size() > 0)
					{
						v.color = p.color * mesh->colors[j];
					}
					else
					{
						v.color = p.color;
					}
				}
			}

			if (render_mode != ParticleSystemRenderMode::Mesh)
			{
				auto& v0 = vs[index * 4 + 0];
				auto& v1 = vs[index * 4 + 1];
				auto& v2 = vs[index * 4 + 2];
				auto& v3 = vs[index * 4 + 3];

				v0.uv = Vector2(p.uv_scale_offset.z, p.uv_scale_offset.y + p.uv_scale_offset.w);
				v1.uv = Vector2(p.uv_scale_offset.x + p.uv_scale_offset.z, p.uv_scale_offset.y + p.uv_scale_offset.w);
				v2.uv = Vector2(p.uv_scale_offset.x + p.uv_scale_offset.z, p.uv_scale_offset.w);
				v3.uv = Vector2(p.uv_scale_offset.z, p.uv_scale_offset.w);
				v0.color = p.color;
				v1.color = p.color;
				v2.color = p.color;
				v3.color = p.color;
			}
		}
	}

	void ParticleSystem::FillIndexBuffer(void* param, const ByteBuffer& buffer)
	{
		auto ps = (ParticleSystem*) param;
		MemoryStream ms(buffer);

		int index_offset = 0;
		for (auto i = ps->m_partices.Begin(); i != ps->m_partices.End(); i = i->next)
		{
			if (ps->m_renderer->render_mode == ParticleSystemRenderMode::Mesh)
			{
				for (auto index : ps->m_renderer->mesh->triangles)
				{
					ms.Write<unsigned short>(index_offset + index);
				}
				index_offset += ps->m_renderer->mesh->vertices.Size();
			}
			else
			{
				ms.Write<unsigned short>(index_offset + 0);
				ms.Write<unsigned short>(index_offset + 1);
				ms.Write<unsigned short>(index_offset + 2);
				ms.Write<unsigned short>(index_offset + 0);
				ms.Write<unsigned short>(index_offset + 2);
				ms.Write<unsigned short>(index_offset + 3);
				index_offset += 4;
			}
		}

		ms.Close();
	}

	void ParticleSystem::UpdateBuffer()
	{
		if (m_partices.Size() == 0)
		{
			return;
		}

		int vertex_count_per_particle = 4;
		int index_count_per_particle = 6;

		if (m_renderer->render_mode == ParticleSystemRenderMode::Mesh)
		{
			vertex_count_per_particle = m_renderer->mesh->vertices.Size();
			index_count_per_particle = m_renderer->mesh->triangles.Size();
		}

		int vertex_count = m_partices.Size() * vertex_count_per_particle;
		assert(vertex_count < 65536);
		int vertex_buffer_size = vertex_count * VERTEX_STRIDE;
		if (!m_vertex_buffer || m_vertex_buffer->GetSize() < vertex_buffer_size)
		{
			m_vertex_buffer = VertexBuffer::Create(vertex_buffer_size, true);
		}
		m_vertex_buffer->Fill(this, ParticleSystem::FillVertexBuffer);

		int index_count = m_partices.Size() * index_count_per_particle;
		int index_buffer_size = index_count * sizeof(unsigned short);
		if (!m_index_buffer || m_index_buffer->GetSize() < index_buffer_size)
		{
			m_index_buffer = IndexBuffer::Create(index_buffer_size, false);
			m_index_buffer->Fill(this, ParticleSystem::FillIndexBuffer);
		}
	}

	void ParticleSystem::GetIndexRange(int submesh_index, int& start, int& count)
	{
		int index_count_per_particle = 6;
		if (m_renderer->render_mode == ParticleSystemRenderMode::Mesh)
		{
			index_count_per_particle = m_renderer->mesh->triangles.Size();
		}

		start = 0;
		count = m_partices.Size() * index_count_per_particle;
	}

	void ParticleSystem::UpdateParticleVelocity(Particle& p)
	{
		Vector3 v = p.start_velocity;
		float delta_time = Time::GetDeltaTime() * main.simulation_speed;
		float lifetime_t = Mathf::Clamp01((p.start_lifetime - p.remaining_lifetime) / p.start_lifetime);

		if (velocity_over_lifetime.enabled)
		{
			float x = velocity_over_lifetime.x.Evaluate(lifetime_t);
			float y = velocity_over_lifetime.y.Evaluate(lifetime_t);
			float z = velocity_over_lifetime.z.Evaluate(lifetime_t);

			if (main.simulation_space == ParticleSystemSimulationSpace::World)
			{
				if (velocity_over_lifetime.space == ParticleSystemSimulationSpace::World)
				{
					v += Vector3(x, y, z);
				}
				else
				{
					v += this->GetTransform()->GetLocalToWorldMatrix().MultiplyDirection(Vector3(x, y, z));
				}
			}
			else
			{
				if (velocity_over_lifetime.space == ParticleSystemSimulationSpace::World)
				{
					v += this->GetTransform()->GetWorldToLocalMatrix().MultiplyDirection(Vector3(x, y, z));
				}
				else
				{
					v += Vector3(x, y, z);
				}
			}
		}

		if (force_over_lifetime.enabled)
		{
			float x = force_over_lifetime.x.Evaluate(lifetime_t);
			float y = force_over_lifetime.y.Evaluate(lifetime_t);
			float z = force_over_lifetime.z.Evaluate(lifetime_t);

			if (main.simulation_space == ParticleSystemSimulationSpace::World)
			{
				if (force_over_lifetime.space == ParticleSystemSimulationSpace::World)
				{
					p.force_velocity += Vector3(x, y, z) * delta_time;
				}
				else
				{
					p.force_velocity += this->GetTransform()->GetLocalToWorldMatrix().MultiplyDirection(Vector3(x, y, z)) * delta_time;
				}
			}
			else
			{
				if (force_over_lifetime.space == ParticleSystemSimulationSpace::World)
				{
					p.force_velocity += this->GetTransform()->GetWorldToLocalMatrix().MultiplyDirection(Vector3(x, y, z)) * delta_time;
				}
				else
				{
					p.force_velocity += Vector3(x, y, z) * delta_time;
				}
			}
		}

		v += p.force_velocity;

		if (limit_velocity_over_lifetime.enabled)
		{
			if (limit_velocity_over_lifetime.separate_axes)
			{
				float x = limit_velocity_over_lifetime.limit_x.Evaluate(lifetime_t);
				float y = limit_velocity_over_lifetime.limit_y.Evaluate(lifetime_t);
				float z = limit_velocity_over_lifetime.limit_z.Evaluate(lifetime_t);
				Vector3 limit;

				if (main.simulation_space == ParticleSystemSimulationSpace::World)
				{
					if (limit_velocity_over_lifetime.space == ParticleSystemSimulationSpace::World)
					{
						limit = Vector3(x, y, z);
					}
					else
					{
						limit = this->GetTransform()->GetLocalToWorldMatrix().MultiplyDirection(Vector3(x, y, z));
					}
				}
				else
				{
					if (limit_velocity_over_lifetime.space == ParticleSystemSimulationSpace::World)
					{
						limit = this->GetTransform()->GetWorldToLocalMatrix().MultiplyDirection(Vector3(x, y, z));
					}
					else
					{
						limit = Vector3(x, y, z);
					}
				}

				if (v.x > limit.x)
				{
					float exceed = v.x - limit.x;
					exceed *= 1.0f - limit_velocity_over_lifetime.dampen;
					v.x = limit.x + exceed;
				}
				if (v.y > limit.y)
				{
					float exceed = v.y - limit.y;
					exceed *= 1.0f - limit_velocity_over_lifetime.dampen;
					v.y = limit.y + exceed;
				}
				if (v.z > limit.z)
				{
					float exceed = v.z - limit.z;
					exceed *= 1.0f - limit_velocity_over_lifetime.dampen;
					v.z = limit.z + exceed;
				}
			}
			else
			{
				float limit = limit_velocity_over_lifetime.limit.Evaluate(lifetime_t);
				float mag = v.Magnitude();
				if (mag > limit)
				{
					float exceed = mag - limit;
					exceed *= 1.0f - limit_velocity_over_lifetime.dampen;
					v = Vector3::Normalize(v) * (limit + exceed);
				}
			}
		}

		p.velocity = v;
	}

	void ParticleSystem::UpdateParticleAngularVelocity(Particle& p)
	{
		Vector3 v = Vector3(0, 0, 0);

		if (rotation_over_lifetime.enabled)
		{
			if (rotation_over_lifetime.z.mode == ParticleSystemCurveMode::TwoConstants)
			{
				v = p.rotation_over_lifetime_random;
			}
			else
			{
				float lifetime_t = Mathf::Clamp01((p.start_lifetime - p.remaining_lifetime) / p.start_lifetime);

				if (rotation_over_lifetime.separate_axes)
				{
					v.x += rotation_over_lifetime.x.Evaluate(lifetime_t);
					v.y += rotation_over_lifetime.y.Evaluate(lifetime_t);
					v.z += rotation_over_lifetime.z.Evaluate(lifetime_t);
				}
				else
				{
					v.z += rotation_over_lifetime.z.Evaluate(lifetime_t);
				}
			}
		}

		if (rotation_by_speed.enabled)
		{
			float speed = p.velocity.Magnitude();
			float speed_t = (speed - color_by_speed.range.x) / (color_by_speed.range.y - color_by_speed.range.x);
			speed_t = Mathf::Clamp01(speed_t);

			if (rotation_by_speed.separate_axes)
			{
				v.x += rotation_by_speed.x.Evaluate(speed_t);
				v.y += rotation_by_speed.y.Evaluate(speed_t);
				v.z += rotation_by_speed.z.Evaluate(speed_t);
			}
			else
			{
				v.z += rotation_by_speed.z.Evaluate(speed_t);
			}
		}

		p.angular_velocity = v;
	}

	void ParticleSystem::UpdateParticleColor(Particle& p)
	{
		Color c = p.start_color;

		if (color_over_lifetime.enabled)
		{
			float lifetime_t = Mathf::Clamp01((p.start_lifetime - p.remaining_lifetime) / p.start_lifetime);
			c *= color_over_lifetime.color.Evaluate(lifetime_t);
		}

		if (color_by_speed.enabled)
		{
			float speed = p.velocity.Magnitude();
			float speed_t = (speed - color_by_speed.range.x) / (color_by_speed.range.y - color_by_speed.range.x);
			speed_t = Mathf::Clamp01(speed_t);

			c *= color_by_speed.color.Evaluate(speed_t);
		}

		p.color = c;
	}

	void ParticleSystem::UpdateParticleSize(Particle& p)
	{
		Vector3 s = p.start_size;

		if (size_over_lifetime.enabled)
		{
			float lifetime_t = Mathf::Clamp01((p.start_lifetime - p.remaining_lifetime) / p.start_lifetime);

			if (size_over_lifetime.separate_axes)
			{
				s.x *= size_over_lifetime.x.Evaluate(lifetime_t);
				s.y *= size_over_lifetime.y.Evaluate(lifetime_t);
				s.z *= size_over_lifetime.z.Evaluate(lifetime_t);
			}
			else
			{
				s *= size_over_lifetime.size.Evaluate(lifetime_t);
			}
		}

		if (size_by_speed.enabled)
		{
			float speed = p.velocity.Magnitude();
			float speed_t = (speed - size_by_speed.range.x) / (size_by_speed.range.y - size_by_speed.range.x);
			speed_t = Mathf::Clamp01(speed_t);

			if (size_by_speed.separate_axes)
			{
				s.x *= size_by_speed.x.Evaluate(speed_t);
				s.y *= size_by_speed.y.Evaluate(speed_t);
				s.z *= size_by_speed.z.Evaluate(speed_t);
			}
			else
			{
				s *= size_by_speed.size.Evaluate(speed_t);
			}
		}

		p.size = s;
	}

	void ParticleSystem::UpdateParticleUV(Particle& p)
	{
		if (texture_sheet_animation.enabled)
		{
			p.uv_scale_offset.x = 1.0f / texture_sheet_animation.num_tiles_x;
			p.uv_scale_offset.y = 1.0f / texture_sheet_animation.num_tiles_y;

			int row = 0;
			int frame_over_time;

			if (texture_sheet_animation.animation == ParticleSystemAnimationType::SingleRow)
			{
				if (texture_sheet_animation.use_random_row)
				{
					row = p.texture_sheet_animation_row;
				}
				else
				{
					row = texture_sheet_animation.row_index;
				}
			}

			if (texture_sheet_animation.frame_over_time.mode == ParticleSystemCurveMode::TwoConstants)
			{
				frame_over_time = p.texture_sheet_animation_frame;
			}
			else
			{
				float lifetime_t = Mathf::Clamp01((p.start_lifetime - p.remaining_lifetime) / p.start_lifetime);
				if (texture_sheet_animation.animation == ParticleSystemAnimationType::SingleRow)
				{
					frame_over_time = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.frame_over_time.Evaluate(lifetime_t));
				}
				else
				{
					frame_over_time = (int) (texture_sheet_animation.num_tiles_x * texture_sheet_animation.num_tiles_y * texture_sheet_animation.frame_over_time.Evaluate(lifetime_t));
				}
			}

			int frame = row * texture_sheet_animation.num_tiles_x + p.texture_sheet_animation_start_frame + frame_over_time;
			frame = frame % (texture_sheet_animation.num_tiles_x * texture_sheet_animation.num_tiles_y);
			int x = frame % texture_sheet_animation.num_tiles_x;
			int y = frame / texture_sheet_animation.num_tiles_x;
			p.uv_scale_offset.z = x * p.uv_scale_offset.x;
			p.uv_scale_offset.w = y * p.uv_scale_offset.y;
		}
		else
		{
			p.uv_scale_offset = Vector4(1, 1, 0, 0);
		}
	}

	void ParticleSystem::UpdateParticlePosition(Particle& p)
	{
		float delta_time = Time::GetDeltaTime() * main.simulation_speed;

		p.position += p.velocity * delta_time;
	}

	void ParticleSystem::UpdateParticleRotation(Particle& p)
	{
		float delta_time = Time::GetDeltaTime() * main.simulation_speed;

		p.rotation += p.angular_velocity * delta_time;
	}

	void ParticleSystem::UpdateParticleLifetime(Particle& p)
	{
		p.remaining_lifetime = p.start_lifetime - (Time::GetTime() - p.emit_time) * main.simulation_speed;
	}

	void ParticleSystem::EmitShapeSphere(Vector3& position, Vector3& velocity, bool hemi)
	{
		Vector3 dir;
		do
		{
			dir.x = Mathf::RandomRange(-0.5f, 0.5f);
			dir.y = Mathf::RandomRange(-0.5f, 0.5f);
			if (hemi)
			{
				dir.z = Mathf::RandomRange(0.0f, 0.5f);
			}
			else
			{
				dir.z = Mathf::RandomRange(-0.5f, 0.5f);
			}
		} while (Mathf::FloatEqual(dir.SqrMagnitude(), 0));
		dir.Normalize();

		float radius = 0;
		switch (shape.shape_type)
		{
			case ParticleSystemShapeType::Sphere:
			case ParticleSystemShapeType::Hemisphere:
				radius = Mathf::RandomRange(0.0f, shape.radius);
				break;
			case ParticleSystemShapeType::SphereShell:
			case ParticleSystemShapeType::HemisphereShell:
				radius = shape.radius;
				break;
            default:
                break;
		}
		position = dir * radius;
		velocity = dir;

		if (shape.random_direction_amount > 0)
		{
			Vector3 dir;
			do
			{
				dir.x = Mathf::RandomRange(-0.5f, 0.5f);
				dir.y = Mathf::RandomRange(-0.5f, 0.5f);
				dir.z = Mathf::RandomRange(-0.5f, 0.5f);
			} while (Mathf::FloatEqual(dir.SqrMagnitude(), 0));

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.random_direction_amount);
			velocity.Normalize();
		}
	}

	void ParticleSystem::EmitShapeCone(Vector3& position, Vector3& velocity)
	{
		float angle = Mathf::Clamp(shape.angle, 1.0f, 89.0f);
		Vector3 origin = Vector3(0, 0, -shape.radius / tanf(angle * Mathf::Deg2Rad));
		float arc = Mathf::RandomRange(0.0f, shape.arc);
		float z = 0;
		float radius = 0;

		switch (shape.shape_type)
		{
			case ParticleSystemShapeType::Cone:
			{
				z = 0;
				radius = Mathf::RandomRange(0.0f, shape.radius);
				break;
			}
			case ParticleSystemShapeType::ConeShell:
			{
				z = 0;
				radius = shape.radius;
				break;
			}
			case ParticleSystemShapeType::ConeVolume:
			{
				z = Mathf::RandomRange(0.0f, shape.length);
				radius = tanf(angle * Mathf::Deg2Rad) / (fabsf(origin.z) + z);
				radius = Mathf::RandomRange(0.0f, radius);
				break;
			}
			case ParticleSystemShapeType::ConeVolumeShell:
			{
				z = Mathf::RandomRange(0.0f, shape.length);
				radius = tanf(angle * Mathf::Deg2Rad) / (fabsf(origin.z) + z);
				break;
			}
            default:
                break;
		}

		float x = radius * cosf(arc * Mathf::Deg2Rad);
		float y = radius * sinf(arc * Mathf::Deg2Rad);
		position = Vector3(x, y, z);
		velocity = Vector3::Normalize(position - origin);

		if (shape.random_direction_amount > 0)
		{
			Vector3 dir;
			float r = Mathf::RandomRange(0.0f, shape.radius);
			Vector3 pos = Vector3(r * cosf(arc * Mathf::Deg2Rad), r * sinf(arc * Mathf::Deg2Rad), 0);
			dir = pos - origin;

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.random_direction_amount);
			velocity.Normalize();
		}

		if (shape.spherical_direction_amount > 0)
		{
			Vector3 dir = position;

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.spherical_direction_amount);
			velocity.Normalize();
		}
	}

	void ParticleSystem::EmitShapeBox(Vector3& position, Vector3& velocity)
	{
		float x = Mathf::RandomRange(-0.5f, 0.5f) * shape.box.x;
		float y = Mathf::RandomRange(-0.5f, 0.5f) * shape.box.y;
		float z = Mathf::RandomRange(-0.5f, 0.5f) * shape.box.z;

		switch (shape.shape_type)
		{
			case ParticleSystemShapeType::Box:
			{
				position.x = x;
				position.y = y;
				position.z = z;
				break;
			}
			case ParticleSystemShapeType::BoxShell:
			{
				int side = Mathf::RandomRange(0, 6);
				switch (side)
				{
					case 0:
						position.x = -0.5f * shape.box.x;
						position.y = y;
						position.z = z;
						break;
					case 1:
						position.x = 0.5f * shape.box.x;
						position.y = y;
						position.z = z;
						break;
					case 2:
						position.x = x;
						position.y = -0.5f * shape.box.y;
						position.z = z;
						break;
					case 3:
						position.x = x;
						position.y = 0.5f * shape.box.y;
						position.z = z;
						break;
					case 4:
						position.x = x;
						position.y = y;
						position.z = -0.5f * shape.box.z;
						break;
					case 5:
						position.x = x;
						position.y = y;
						position.z = 0.5f * shape.box.z;
						break;
				}
				break;
			}
			case ParticleSystemShapeType::BoxEdge:
			{
				int edge = Mathf::RandomRange(0, 12);
				switch (edge)
				{
					case 0:
						position.x = -0.5f * shape.box.x;
						position.y = -0.5f * shape.box.y;
						position.z = z;
						break;
					case 1:
						position.x = -0.5f * shape.box.x;
						position.y = 0.5f * shape.box.y;
						position.z = z;
						break;
					case 2:
						position.x = 0.5f * shape.box.x;
						position.y = -0.5f * shape.box.y;
						position.z = z;
						break;
					case 3:
						position.x = 0.5f * shape.box.x;
						position.y = 0.5f * shape.box.y;
						position.z = z;
						break;
					case 4:
						position.x = x;
						position.y = -0.5f * shape.box.y;
						position.z = -0.5f * shape.box.z;
						break;
					case 5:
						position.x = x;
						position.y = -0.5f * shape.box.y;
						position.z = 0.5f * shape.box.z;
						break;
					case 6:
						position.x = x;
						position.y = 0.5f * shape.box.y;
						position.z = -0.5f * shape.box.z;
						break;
					case 7:
						position.x = x;
						position.y = 0.5f * shape.box.y;
						position.z = 0.5f * shape.box.z;
						break;
					case 8:
						position.x = -0.5f * shape.box.x;
						position.y = y;
						position.z = -0.5f * shape.box.z;
						break;
					case 9:
						position.x = -0.5f * shape.box.x;
						position.y = y;
						position.z = 0.5f * shape.box.z;
						break;
					case 10:
						position.x = 0.5f * shape.box.x;
						position.y = y;
						position.z = -0.5f * shape.box.z;
						break;
					case 11:
						position.x = 0.5f * shape.box.x;
						position.y = y;
						position.z = 0.5f * shape.box.z;
						break;
				}
				break;
			}
            default:
                break;
		}

		if (shape.random_direction_amount > 0)
		{
			Vector3 dir;
			do
			{
				dir.x = Mathf::RandomRange(-0.5f, 0.5f);
				dir.y = Mathf::RandomRange(-0.5f, 0.5f);
				dir.z = Mathf::RandomRange(-0.5f, 0.5f);
			} while (Mathf::FloatEqual(dir.SqrMagnitude(), 0));

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.random_direction_amount);
			velocity.Normalize();
		}

		if (shape.spherical_direction_amount > 0)
		{
			Vector3 dir = position;

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.spherical_direction_amount);
			velocity.Normalize();
		}
	}

	void ParticleSystem::EmitShapeCircle(Vector3& position, Vector3& velocity)
	{
		float arc = Mathf::RandomRange(0.0f, shape.arc);
		float radius = 0;

		switch (shape.shape_type)
		{
			case ParticleSystemShapeType::Circle:
			{
				radius = Mathf::RandomRange(0.0f, shape.radius);
				break;
			}
			case ParticleSystemShapeType::CircleEdge:
			{
				radius = shape.radius;
				break;
			}
            default:
                break;
		}

		float x = radius * cosf(arc * Mathf::Deg2Rad);
		float y = radius * sinf(arc * Mathf::Deg2Rad);
		position = Vector3(x, y, 0);
		velocity = Vector3::Normalize(position);

		if (shape.random_direction_amount > 0)
		{
			Vector3 dir;
			do
			{
				dir.x = Mathf::RandomRange(-0.5f, 0.5f);
				dir.y = Mathf::RandomRange(-0.5f, 0.5f);
				dir.z = 0;
			} while (Mathf::FloatEqual(dir.SqrMagnitude(), 0));

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.random_direction_amount);
			velocity.Normalize();
		}
	}

	void ParticleSystem::EmitShapeEdge(Vector3& position, Vector3& velocity)
	{
		float x = Mathf::RandomRange(-1.0f, 1.0f) * shape.radius;

		position = Vector3(x, 0, 0);
		velocity = Vector3(0, 1, 0);

		if (shape.random_direction_amount > 0)
		{
			Vector3 dir;
			do
			{
				dir.x = Mathf::RandomRange(-0.5f, 0.5f);
				dir.y = Mathf::RandomRange(-0.5f, 0.5f);
				dir.z = Mathf::RandomRange(-0.5f, 0.5f);
			} while (Mathf::FloatEqual(dir.SqrMagnitude(), 0));

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.random_direction_amount);
			velocity.Normalize();
		}

		if (shape.spherical_direction_amount > 0)
		{
			Vector3 dir = position;

			velocity = Vector3::Lerp(velocity, Vector3::Normalize(dir), shape.spherical_direction_amount);
			velocity.Normalize();
		}
	}

	float ParticleSystem::MinMaxCurve::Evaluate(float time)
	{
		time = Mathf::Clamp01(time);

		if (mode == ParticleSystemCurveMode::Constant)
		{
			return constant;
		}
		else if (mode == ParticleSystemCurveMode::TwoConstants)
		{
			return Mathf::RandomRange(constant_min, constant_max);
		}
		else if (mode == ParticleSystemCurveMode::Curve)
		{
			return curve.Evaluate(time);
		}
		else if (mode == ParticleSystemCurveMode::TwoCurves)
		{
			return Mathf::RandomRange(curve_min.Evaluate(time), curve_max.Evaluate(time));
		}

		return 0;
	}

	Color Gradient::Evaluate(float time)
	{
		Color c;

		if (time <= r.keys[0].time)
		{
			c.r = r.keys[0].value;
			c.g = g.keys[0].value;
			c.b = b.keys[0].value;
		}
		else if (time >= r.keys[r.keys.Size() - 1].time)
		{
			c.r = r.keys[r.keys.Size() - 1].value;
			c.g = g.keys[r.keys.Size() - 1].value;
			c.b = b.keys[r.keys.Size() - 1].value;
		}
		else
		{
			int index;

			for (int i = 0; i < r.keys.Size() - 1; i++)
			{
				if (time > r.keys[i].time && time <= r.keys[i + 1].time)
				{
					index = i;
					break;
				}
			}

			if (mode == GradientMode::Blend)
			{
				float t = (time - r.keys[index].time) / (r.keys[index + 1].time - r.keys[index].time);
				c.r = Mathf::Lerp(r.keys[index].value, r.keys[index + 1].value, t);
				c.g = Mathf::Lerp(g.keys[index].value, g.keys[index + 1].value, t);
				c.b = Mathf::Lerp(b.keys[index].value, b.keys[index + 1].value, t);
			}
			else if (mode == GradientMode::Fixed)
			{
				c.r = r.keys[index + 1].value;
				c.g = g.keys[index + 1].value;
				c.b = b.keys[index + 1].value;
			}
		}

		if (time <= a.keys[0].time)
		{
			c.a = a.keys[0].value;
		}
		else if (time >= a.keys[a.keys.Size() - 1].time)
		{
			c.a = a.keys[a.keys.Size() - 1].value;
		}
		else
		{
			int index;

			for (int i = 0; i < a.keys.Size() - 1; i++)
			{
				if (time > a.keys[i].time && time <= a.keys[i + 1].time)
				{
					index = i;
					break;
				}
			}

			if (mode == GradientMode::Blend)
			{
				float t = (time - a.keys[index].time) / (a.keys[index + 1].time - a.keys[index].time);
				c.a = Mathf::Lerp(a.keys[index].value, a.keys[index + 1].value, t);
			}
			else if (mode == GradientMode::Fixed)
			{
				c.a = a.keys[index + 1].value;
			}
		}

		return c;
	}

	Color ParticleSystem::MinMaxGradient::Evaluate(float time)
	{
		time = Mathf::Clamp01(time);

		if (mode == ParticleSystemGradientMode::Color)
		{
			return color;
		}
		else if (mode == ParticleSystemGradientMode::Gradient)
		{
			return gradient.Evaluate(time);
		}
		else if (mode == ParticleSystemGradientMode::TwoColors)
		{
			float r = Mathf::RandomRange(color_min.r, color_max.r);
			float g = Mathf::RandomRange(color_min.g, color_max.g);
			float b = Mathf::RandomRange(color_min.b, color_max.b);
			float a = Mathf::RandomRange(color_min.a, color_max.a);
			return Color(r, g, b, a);
		}
		else if (mode == ParticleSystemGradientMode::TwoGradients)
		{
			Color min = gradient_min.Evaluate(time);
			Color max = gradient_max.Evaluate(time);
			float r = Mathf::RandomRange(min.r, max.r);
			float g = Mathf::RandomRange(min.g, max.g);
			float b = Mathf::RandomRange(min.b, max.b);
			float a = Mathf::RandomRange(min.a, max.a);
			return Color(r, g, b, a);
		}
		else if (mode == ParticleSystemGradientMode::RandomColor)
		{
			return gradient.Evaluate(Mathf::RandomRange(0.0f, 1.0f));
		}

		return Color();
	}
}
