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

#pragma once

#include "Renderer.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
	enum class ParticleSystemRenderMode
	{
		Billboard = 0,
		Stretch = 1,
		HorizontalBillboard = 2,
		VerticalBillboard = 3,
		Mesh = 4,
		None = 5,
	};

	enum class ParticleSystemSortMode
	{
		None = 0,
		Distance = 1,
		OldestInFront = 2,
		YoungestInFront = 3
	};

	enum class ParticleSystemRenderSpace
	{
		View = 0,
		World = 1,
		Local = 2,
		Facing = 3
	};

	class ParticleSystem;

	class ParticleSystemRenderer: public Renderer
	{
		DECLARE_COM_CLASS(ParticleSystemRenderer, Renderer);
	public:
		virtual const VertexBuffer* GetVertexBuffer() const;
		virtual const IndexBuffer* GetIndexBuffer() const;
		virtual void GetIndexRange(int material_index, int& start, int& count) const;

	public:
		ParticleSystemRenderMode render_mode;
		float camera_velocity_scale;
		float velocity_scale;
		float length_scale;
		Ref<Mesh> mesh;
		float normal_direction;
		ParticleSystemSortMode sort_mode;
		float sorting_fudge;
		float min_particle_size;
		float max_particle_size;
		ParticleSystemRenderSpace alignment;
		Vector3 pivot;

	protected:
		virtual void Start();
		virtual void PreRenderByRenderer(int material_index);
		virtual Matrix4x4 GetWorldMatrix();

	private:
		ParticleSystemRenderer();

	private:
		WeakRef<ParticleSystem> m_particle_system;
	};
}
