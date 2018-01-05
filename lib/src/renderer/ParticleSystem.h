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

#include "Component.h"
#include "animation/AnimationCurve.h"
#include "graphics/Color.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"
#include "container/Vector.h"
#include "container/FastList.h"

namespace Viry3D
{
	enum class ParticleSystemCurveMode
	{
		None = -1,
		Constant = 0,
		Curve = 1,
		TwoCurves = 2,
		TwoConstants = 3,
	};

	enum class ParticleSystemGradientMode
	{
		None = -1,
		Color = 0,
		Gradient = 1,
		TwoColors = 2,
		TwoGradients = 3,
		RandomColor = 4,
	};

	enum class ParticleSystemSimulationSpace
	{
		Local = 0,
		World = 1,
		Custom = 2, // no implement
	};

	enum class ParticleSystemScalingMode
	{
		Hierarchy = 0,
		Local = 1,
		Shape = 2,
	};

	enum class ParticleSystemShapeType
	{
		Sphere = 0,
		SphereShell = 1,
		Hemisphere = 2,
		HemisphereShell = 3,
		Cone = 4,
		Box = 5,
		Mesh = 6,
		ConeShell = 7,
		ConeVolume = 8,
		ConeVolumeShell = 9,
		Circle = 10,
		CircleEdge = 11,
		SingleSidedEdge = 12,
		MeshRenderer = 13,
		SkinnedMeshRenderer = 14,
		BoxShell = 15,
		BoxEdge = 16,
	};

	enum class ParticleSystemShapeMultiModeValue
	{
		Random = 0,
		Loop = 1,
		PingPong = 2,
		BurstSpread = 3,
	};

	enum class ParticleSystemInheritVelocityMode
	{
		Initial = 0,
		Current = 1,
	};

	enum class ParticleSystemAnimationType
	{
		WholeSheet = 0,
		SingleRow = 1,
	};

	enum class GradientMode
	{
		None = -1,
		Blend = 0,
		Fixed = 1,
	};

	struct Gradient
	{
		GradientMode mode;
		AnimationCurve r;
		AnimationCurve g;
		AnimationCurve b;
		AnimationCurve a;

		Gradient(): mode(GradientMode::None) { }
		Color Evaluate(float time);
	};

	class ParticleSystemRenderer;

	class ParticleSystem: public Component
	{
		DECLARE_COM_CLASS(ParticleSystem, Component);
	public:
		friend class ParticleSystemRenderer;

		struct MinMaxCurve
		{
			ParticleSystemCurveMode mode;
			float constant;
			AnimationCurve curve;
			AnimationCurve curve_min;
			AnimationCurve curve_max;
			float constant_min;
			float constant_max;
			float curve_multiplier;

			MinMaxCurve(): mode(ParticleSystemCurveMode::None) { }
			float Evaluate(float time, float lerp);
		};

		struct MinMaxGradient
		{
			ParticleSystemGradientMode mode;
			Color color;
			Gradient gradient;
			Color color_min;
			Color color_max;
			Gradient gradient_min;
			Gradient gradient_max;

			MinMaxGradient(): mode(ParticleSystemGradientMode::None) { }
			Color Evaluate(float time, float lerp);
		};

		struct MainModule
		{
			float duration;
			bool loop;
			MinMaxCurve start_delay;
			MinMaxCurve start_lifetime;
			MinMaxCurve start_speed;
			bool start_size_3d;
			MinMaxCurve start_size_x;
			MinMaxCurve start_size_y;
			MinMaxCurve start_size_z;
			MinMaxCurve start_size;
			bool start_rotation_3d;
			MinMaxCurve start_rotation_x;
			MinMaxCurve start_rotation_y;
			MinMaxCurve start_rotation_z;
			MinMaxCurve start_rotation;
			float randomize_rotation_direction;
			MinMaxGradient start_color;
			MinMaxCurve gravity_modifier;
			ParticleSystemSimulationSpace simulation_space;
			float simulation_speed;
			ParticleSystemScalingMode scaling_mode;
			int max_particles;
		};

		struct EmissionBurst
		{
			float time;
			int min_count;
			int max_count;
			int cycle_count;
			float repeat_interval;

			float emit_time;
			int emit_count;
			EmissionBurst(): emit_time(-1), emit_count(0) { }
		};

		struct EmissionModule
		{
			bool enabled;
			MinMaxCurve rate_over_time;
            float rate_over_time_lerp = -1;
			MinMaxCurve rate_over_distance;
			Vector<EmissionBurst> bursts;

			void ResetBurstState()
			{
				for (auto& i : bursts)
				{
					i.emit_time = -1;
					i.emit_count = 0;
				}
			}
		};

		struct ShapeModule
		{
			bool enabled;
			ParticleSystemShapeType shape_type;
			float radius;
			float angle;
			float arc;
			ParticleSystemShapeMultiModeValue arc_mode;
			float arc_spread;
			MinMaxCurve arc_speed;
			float length;
			Vector3 box;
			ParticleSystemShapeMultiModeValue radius_mode;
			float radius_spread;
			MinMaxCurve radius_speed;
			bool align_to_direction;
			float random_direction_amount;
			float spherical_direction_amount;
		};

		struct VelocityOverLifetimeModule
		{
			bool enabled;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
			ParticleSystemSimulationSpace space;
		};

		struct LimitVelocityOverLifetimeModule
		{
			bool enabled;
			bool separate_axes;
			MinMaxCurve limit_x;
			MinMaxCurve limit_y;
			MinMaxCurve limit_z;
			ParticleSystemSimulationSpace space;
			MinMaxCurve limit;
			float dampen;
		};

		struct InheritVelocityModule
		{
			bool enabled;
			ParticleSystemInheritVelocityMode mode;
			MinMaxCurve curve;
		};

		struct ForceOverLifetimeModule
		{
			bool enabled;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
			ParticleSystemSimulationSpace space;
			bool randomized;
		};

		struct ColorOverLifetimeModule
		{
			bool enabled;
			MinMaxGradient color;
		};

		struct ColorBySpeedModule
		{
			bool enabled;
			MinMaxGradient color;
			Vector2 range;
		};

		struct SizeOverLifetimeModule
		{
			bool enabled;
			bool separate_axes;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
			MinMaxCurve size;
		};

		struct SizeBySpeedModule
		{
			bool enabled;
			bool separate_axes;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
			MinMaxCurve size;
			Vector2 range;
		};

		struct RotationOverLifetimeModule
		{
			bool enabled;
			bool separate_axes;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
		};

		struct RotationBySpeedModule
		{
			bool enabled;
			bool separate_axes;
			MinMaxCurve x;
			MinMaxCurve y;
			MinMaxCurve z;
			Vector2 range;
		};

		struct ExternalForcesModule
		{
			bool enabled;
			float multiplier;
		};

		struct TextureSheetAnimationModule
		{
			bool enabled;
			int num_tiles_x;
			int num_tiles_y;
			ParticleSystemAnimationType animation;
			bool use_random_row;
			int row_index;
			MinMaxCurve frame_over_time;
			MinMaxCurve start_frame;
			int cycle_count;
			float flip_u;
			float flip_v;
			int uv_channel_mask;
		};

		struct Particle
		{
			float start_lifetime;
			float remaining_lifetime;
			Vector3 start_size;
			Color start_color;
			Vector3 start_velocity;
			Vector3 force_velocity;
			Vector3 velocity;
			Vector3 angular_velocity;
			Color color;
			Vector3 size;
			Vector3 position;
			Vector3 rotation;

			float velocity_over_lifetime_x_lerp = -1;
			float velocity_over_lifetime_y_lerp = -1;
			float velocity_over_lifetime_z_lerp = -1;
			float force_over_lifetime_x_lerp = -1;
			float force_over_lifetime_y_lerp = -1;
			float force_over_lifetime_z_lerp = -1;
			float limit_velocity_over_lifetime_limit_x_lerp = -1;
			float limit_velocity_over_lifetime_limit_y_lerp = -1;
			float limit_velocity_over_lifetime_limit_z_lerp = -1;
			float limit_velocity_over_lifetime_limit_lerp = -1;
			float rotation_over_lifetime_x_lerp = -1;
			float rotation_over_lifetime_y_lerp = -1;
			float rotation_over_lifetime_z_lerp = -1;
			float rotation_by_speed_x_lerp = -1;
			float rotation_by_speed_y_lerp = -1;
			float rotation_by_speed_z_lerp = -1;
			float color_over_lifetime_color_lerp = -1;
			float color_by_speed_color_lerp = -1;
			float size_over_lifetime_x_lerp = -1;
			float size_over_lifetime_y_lerp = -1;
			float size_over_lifetime_z_lerp = -1;
			float size_over_lifetime_size_lerp = -1;
			float size_by_speed_x_lerp = -1;
			float size_by_speed_y_lerp = -1;
			float size_by_speed_z_lerp = -1;
			float size_by_speed_size_lerp = -1;
			float texture_sheet_animation_start_frame_lerp = -1;
			float texture_sheet_animation_frame_over_time_lerp = -1;

			float emit_time;
			Vector4 uv_scale_offset;
			int texture_sheet_animation_row;

			Particle(): start_lifetime(0), remaining_lifetime(0) { }
		};

	public:
		virtual ~ParticleSystem();
		int GetParticleCount() const;
		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_vertex_buffer; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_index_buffer; }
		void GetIndexRange(int submesh_index, int& start, int& count);

	public:
		MainModule main;
		EmissionModule emission;
		ShapeModule shape;
		VelocityOverLifetimeModule velocity_over_lifetime;
		LimitVelocityOverLifetimeModule limit_velocity_over_lifetime;
		InheritVelocityModule inherit_velocity; // 无实现
		ForceOverLifetimeModule force_over_lifetime;
		ColorOverLifetimeModule color_over_lifetime;
		ColorBySpeedModule color_by_speed;
		SizeOverLifetimeModule size_over_lifetime;
		SizeBySpeedModule size_by_speed;
		RotationOverLifetimeModule rotation_over_lifetime;
		RotationBySpeedModule rotation_by_speed;
		ExternalForcesModule external_forces; // 影响风区，无实现
		TextureSheetAnimationModule texture_sheet_animation;

	protected:
		virtual void Start();
		virtual void Update();

	private:
		static void FillVertexBuffer(void* param, const ByteBuffer& buffer);
		static void FillIndexBuffer(void* param, const ByteBuffer& buffer);

		ParticleSystem();

		void EmitShapeSphere(Vector3 &position, Vector3 &velocity, bool hemi);
		void EmitShapeCone(Vector3 &position, Vector3 &velocity);
		void EmitShapeBox(Vector3 &position, Vector3 &velocity);
		void EmitShapeCircle(Vector3 &position, Vector3 &velocity);
		void EmitShapeEdge(Vector3 &position, Vector3 &velocity);
		void Emit(Particle& p);

		void UpdateParticleVelocity(Particle& p);
		void UpdateParticleAngularVelocity(Particle& p);
		void UpdateParticleColor(Particle& p);
		void UpdateParticleSize(Particle& p);
		void UpdateParticleUV(Particle& p);
		void UpdateParticlePosition(Particle& p);
		void UpdateParticleRotation(Particle& p);
		void UpdateParticleLifetime(Particle& p);

		bool CheckTime();
		void UpdateEmission();
		void UpdateParticles();
		void UpdateBuffer();

	private:
		FastList<Particle> m_partices;
		float m_time_start;
		float m_start_delay;
		float m_time;
		float m_time_emit;
		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
		Ref<ParticleSystemRenderer> m_renderer;
	};
}
