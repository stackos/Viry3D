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

	class ParticleSystemRenderer : public Renderer
	{
		DECLARE_COM_CLASS(ParticleSystemRenderer, Renderer);
	public:
		virtual const VertexBuffer* GetVertexBuffer();
		virtual const IndexBuffer* GetIndexBuffer();
		virtual void GetIndexRange(int material_index, int& start, int& count);

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