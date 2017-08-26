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

	const VertexBuffer* ParticleSystemRenderer::GetVertexBuffer()
	{
		return m_particle_system.lock()->GetVertexBuffer().get();
	}

	const IndexBuffer* ParticleSystemRenderer::GetIndexBuffer()
	{
		return m_particle_system.lock()->GetIndexBuffer().get();
	}

	void ParticleSystemRenderer::GetIndexRange(int material_index, int& start, int& count)
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
