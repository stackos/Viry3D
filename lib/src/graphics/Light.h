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

#pragma once

#include "Component.h"
#include "container/List.h"
#include "Color.h"
#include "math/Matrix4x4.h"
#include "private/backend/DriverApi.h"

namespace Viry3D
{
    enum class LightType
    {
        Directional,
        Spot,
        Point,
    };

	class Renderer;
	class Texture;
    
    class Light : public Component
    {
    public:
		static const List<Light*>& GetLights() { return m_lights; }
		static const Color& GetAmbientColor() { return m_ambient_color; }
		static void SetAmbientColor(const Color& color);
		static void RenderShadowMaps();
		Light();
        virtual ~Light();
		LightType GetType() const { return m_type; }
		void SetType(LightType type);
		const Color& GetColor() const { return m_color; }
		void SetColor(const Color& color);
		float GetIntensity() const { return m_intensity; }
		void SetIntensity(float intensity);
		float GetRange() const { return m_range; }
		void SetRange(float range);
		float GetSpotAngle() const { return m_spot_angle; }
		void SetSpotAngle(float angle);
		bool IsShadowEnable() const { return m_shadow_enable; }
		void EnableShadow(bool enable);
		void SetShadowTextureSize(int size);
		const Ref<Texture>& GetShadowTexture() const { return m_shadow_texture; }
		void SetShadowStrength(float strength);
		void SetShadowZBias(float bias);
		void SetShadowSlopeBias(float bias);
		void SetNearClip(float clip);
		void SetFarClip(float clip);
		void SetOrthographicSize(float size);
		uint32_t GetCullingMask() const { return m_culling_mask; }
		void SetCullingMask(uint32_t mask);
		const filament::backend::UniformBufferHandle& GetViewUniformBuffer() const { return m_view_uniform_buffer; }
		const filament::backend::UniformBufferHandle& GetLightUniformBuffer() const { return m_light_uniform_buffer; }
		const filament::backend::SamplerGroupHandle& GetSamplerGroup() const { return m_sampler_group; }

	protected:
		virtual void OnTransformDirty();

	private:
		const Matrix4x4& GetViewMatrix();
		const Matrix4x4& GetProjectionMatrix();
		void CullRenderers(const List<Renderer*>& renderers, List<Renderer*>& result);
		void UpdateViewUniforms();
		void Draw(const List<Renderer*>& renderers);
		void DrawRenderer(Renderer* renderer);
		void Prepare();

	private:
		friend class Camera;

    private:
		static List<Light*> m_lights;
		static Color m_ambient_color;
		bool m_dirty;
        LightType m_type;
		Color m_color;
		float m_intensity;
		float m_range;
		float m_spot_angle;
		bool m_shadow_enable;
		int m_shadow_texture_size;
		Ref<Texture> m_shadow_texture;
		float m_shadow_strength;
		float m_shadow_z_bias;
		float m_shadow_slope_bias;
		float m_near_clip;
		float m_far_clip;
		float m_orthographic_size;
		Matrix4x4 m_view_matrix;
		bool m_view_matrix_dirty;
		Matrix4x4 m_projection_matrix;
		bool m_projection_matrix_dirty;
		uint32_t m_culling_mask;
		filament::backend::UniformBufferHandle m_view_uniform_buffer;
		filament::backend::UniformBufferHandle m_light_uniform_buffer;
		filament::backend::SamplerGroupHandle m_sampler_group;
		filament::backend::RenderTargetHandle m_render_target;
    };
}
