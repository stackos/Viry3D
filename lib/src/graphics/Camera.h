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
#include "CameraClearFlags.h"
#include "math/Matrix4x4.h"
#include "math/Rect.h"
#include "math/Frustum.h"
#include "Color.h"
#include "container/List.h"
#include "FrameBuffer.h"

namespace Viry3D
{
	class RenderPass;

	class Camera: public Component
	{
		DECLARE_COM_CLASS(Camera, Component);

	public:
		static void Init();
		static void Deinit();
		static void PrepareAll();
		static void RenderAll();
		static Camera* Current() { return m_current; }
		static bool IsValidCamera(Camera* cam);
		static void OnResize(int width, int height);

		virtual ~Camera();
		int GetDepth() const { return m_depth; }
		void SetDepth(int depth);
		CameraClearFlags GetClearFlags() const { return m_clear_flags; }
		void SetClearFlags(CameraClearFlags flags) { m_clear_flags = flags; }
		const Color& GetClearColor() const { return m_clear_color; }
		void SetClearColor(const Color& color) { m_clear_color = color; }
		bool IsOrthographic() const { return m_orthographic; }
		void SetOrthographic(bool value) { m_orthographic = value; }
		float GetOrthographicSize() const { return m_orthographic_size; }
		void SetOrthographicSize(float size) { m_orthographic_size = size; }
		float GetFieldOfView() const { return m_field_of_view; }
		void SetFieldOfView(float fov) { m_field_of_view = fov; }
		float GetClipNear() const { return m_clip_near; }
		void SetClipNear(float value) { m_clip_near = value; }
		float GetClipFar() const { return m_clip_far; }
		void SetClipFar(float value) { m_clip_far = value; }
		const Rect& GetRect() const { return m_rect; }
		void SetRect(const Rect& rect) { m_rect = rect; }
		int GetCullingMask() const { return m_culling_mask; }
		void SetCullingMask(int mask);
		bool IsHdr() const { return m_hdr; }
		void SetHdr(bool hdr) { m_hdr = hdr; }
		bool CanRender() const;
		bool IsCulling(const Ref<GameObject>& obj) const;
		const Matrix4x4& GetViewMatrix();
		const Matrix4x4& GetProjectionMatrix();
		const Matrix4x4& GetViewProjectionMatrix();
		const Frustum& GetFrustum();
		void SetFrameBuffer(const Ref<FrameBuffer>& frame_buffer);
		int GetTargetWidth() const;
		int GetTargetHeight() const;
		void SetAlwaysRebuildCmd() { m_always_rebuild_cmd = true; }

	protected:
		virtual void OnTranformChanged();

	private:
		static bool Less(const Camera *c1, const Camera *c2);

		Camera();
		void Prepare();
		void Render();
		void DecideTarget();
		void PostProcess();
		void UpdateMatrix();
		Ref<FrameBuffer> GetPostTargetFront();
		Ref<FrameBuffer> GetPostTargetBack();
		void SwapPostTargets();

		static List<Camera*> m_cameras;
		static Camera* m_current;
		static int m_current_index;
		static Ref<FrameBuffer> m_post_target_front;
		static Ref<FrameBuffer> m_post_target_back;

		int m_depth;
		CameraClearFlags m_clear_flags;
		Color m_clear_color;
		bool m_orthographic;
		float m_orthographic_size;
		float m_field_of_view;
		float m_clip_near;
		float m_clip_far;
		Rect m_rect;
		int m_culling_mask;
		bool m_hdr;

		Ref<FrameBuffer> m_frame_buffer;
		Ref<FrameBuffer> m_target_rendering;
		bool m_matrix_dirty;
		Matrix4x4 m_view_matrix;
		Matrix4x4 m_projection_matrix;
		Matrix4x4 m_view_projection_matrix;
		Frustum m_frustum;
		Ref<RenderPass> m_render_pass;
		bool m_always_rebuild_cmd;
	};
}
