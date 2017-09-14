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

#include "ParticleSystemRenderer.h"
#include "ParticleSystem.h"
#include "GameObject.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(ParticleSystemRenderer);

	ParticleSystemRenderer::ParticleSystemRenderer()
	{
	}

	void ParticleSystemRenderer::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<ParticleSystemRenderer>(source);
	}

	const VertexBuffer* ParticleSystemRenderer::GetVertexBuffer() const
	{
		return m_particle_system.lock()->GetVertexBuffer().get();
	}

	const IndexBuffer* ParticleSystemRenderer::GetIndexBuffer() const
	{
		return m_particle_system.lock()->GetIndexBuffer().get();
	}

	void ParticleSystemRenderer::GetIndexRange(int material_index, int& start, int& count) const
	{
		m_particle_system.lock()->GetIndexRange(material_index, start, count);
	}

	void ParticleSystemRenderer::Start()
	{
		Renderer::Start();

		m_particle_system = this->GetGameObject()->GetComponent<ParticleSystem>();
	}

	void ParticleSystemRenderer::PreRenderByRenderer(int material_index)
	{
		Renderer::PreRenderByRenderer(material_index);

		m_particle_system.lock()->UpdateBuffer();
	}

	Matrix4x4 ParticleSystemRenderer::GetWorldMatrix()
	{
		return Matrix4x4::Identity();
	}
}
