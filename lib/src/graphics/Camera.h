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
#include "CameraClearFlags.h"
#include "Color.h"
#include "math/Rect.h"
#include "math/Matrix4x4.h"
#include "container/List.h"
#include "private/backend/DriverApi.h"

namespace Viry3D
{
	class Texture;
    class Renderer;

    class Camera : public Component
    {
    public:
		static void RenderAll();
        static void OnResizeAll(int width, int height);
		Camera();
        virtual ~Camera();
		int GetDepth() const { return m_depth; }
		void SetDepth(int depth);
        uint32_t GetCullingMask() const { return m_culling_mask; }
        void SetCullingMask(uint32_t mask);
		CameraClearFlags GetClearFlags() const { return m_clear_flags; }
		void SetClearFlags(CameraClearFlags flags);
		const Color& GetClearColor() const { return m_clear_color; }
		void SetClearColor(const Color& color);
		const Rect& GetViewportRect() const { return m_viewport_rect; }
		void SetViewportRect(const Rect& rect);
		float GetFieldOfView() const { return m_field_of_view; }
		void SetFieldOfView(float fov);
		float GetNearClip() const { return m_near_clip; }
		void SetNearClip(float clip);
		float GetFarClip() const { return m_far_clip; }
		void SetFarClip(float clip);
		bool IsOrthographic() const { return m_orthographic; }
		void SetOrthographic(bool enable);
		float GetOrthographicSize() const { return m_orthographic_size; }
		void SetOrthographicSize(float size);
		const Matrix4x4& GetViewMatrix();
		const Matrix4x4& GetProjectionMatrix();
		void SetViewMatrixExternal(const Matrix4x4& mat);
		void SetProjectionMatrixExternal(const Matrix4x4& mat);
		const Ref<Texture>& GetRenderTargetColor() const { return m_render_target_color; }
		const Ref<Texture>& GetRenderTargetDepth() const { return m_render_target_depth; }
		void SetRenderTarget(const Ref<Texture>& color, const Ref<Texture>& depth);
		int GetTargetWidth() const;
		int GetTargetHeight() const;

	private:
        void OnResize(int width, int height);
        void CullRenderers(const List<Renderer*>& renderers, List<Renderer*>& result);
        void Prepare(const List<Renderer*>& renderers);
		void UpdateViewUniforms();
        void PrepareRenderer(Renderer* renderer);
		void Draw(const List<Renderer*>& renderers);
        void DrawRenderer(Renderer* renderer);

	private:
		static List<Camera*> m_cameras;
		static bool m_cameras_order_dirty;
		int m_depth;
        uint32_t m_culling_mask;
		CameraClearFlags m_clear_flags;
		Color m_clear_color;
		Rect m_viewport_rect;
		float m_field_of_view;
		float m_near_clip;
		float m_far_clip;
		bool m_orthographic;
		float m_orthographic_size;
		Matrix4x4 m_view_matrix;
		bool m_view_matrix_dirty;
		Matrix4x4 m_projection_matrix;
		bool m_projection_matrix_dirty;
		bool m_view_matrix_external;
		bool m_projection_matrix_external;
		Ref<Texture> m_render_target_color;
		Ref<Texture> m_render_target_depth;
		filament::backend::UniformBufferHandle m_view_uniform_buffer;
		filament::backend::RenderTargetHandle m_render_target;
    };
}
