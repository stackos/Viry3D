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
#include "container/Vector.h"
#include "container/List.h"
#include "container/FastList.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexAttribute.h"
#include "math/Vector4.h"
#include "math/Bounds.h"
#include "math/Matrix4x4.h"
#include "thread/Thread.h"

namespace Viry3D
{
	class Material;
	class Camera;

	class Renderer: public Component
	{
		DECLARE_COM_CLASS_ABSTRACT(Renderer, Component);
	public:
		static void Init();
		static void Deinit();
		static void OnResize(int width, int height);
		static bool IsRenderersDirty();
		static void SetRenderersDirty(bool dirty);
		static void ClearPasses();
		static void SetCullingDirty(Camera* cam);
		static List<Renderer*>& GetRenderers();
		static void PrepareAllPass();
		static void CheckBufferDirty();
		static void RenderAllPass();
		static void HandleUIEvent();
		static void BuildStaticBatch(const Ref<GameObject>& obj);

		Ref<Material> GetSharedMaterial() const;
		void SetSharedMaterial(const Ref<Material>& mat);
		virtual ~Renderer();
		virtual const VertexBuffer* GetVertexBuffer() = 0;
		virtual const IndexBuffer* GetIndexBuffer() = 0;
		virtual const Vector<VertexAttributeOffset>& GetVertexAttributeOffsets() const = 0;
		virtual void GetIndexRange(int material_index, int& start, int& count) = 0;
		virtual bool IsValidPass(int material_index) { return true; }
		const Vector<Ref<Material>>& GetSharedMaterials() const { return m_shared_materials; }
		void SetSharedMaterials(const Vector<Ref<Material>>& mats) { m_shared_materials = mats; }
		int GetSortingOrder() const { return m_sorting_order; }
		void SetSortingOrder(int order) { m_sorting_order = order; }
		int GetLightmapIndex() const { return m_lightmap_index; }
		void SetLightmapIndex(int index) { m_lightmap_index = index; }
		const Vector4& GetLightmapScaleOffset() const { return m_lightmap_scale_offset; }
		void SetLightmapScaleOffset(const Vector4& scale_offset) { m_lightmap_scale_offset = scale_offset; }
		void SetBounds(const Bounds& bounds) { m_bounds = bounds; }
		const Bounds& GetBounds() const { return m_bounds; }

	protected:
		Renderer();
		virtual void Start();
		virtual void OnEnable();
		virtual void OnDisable();
		virtual void PreRenderByMaterial(int material_index);
		virtual void PreRenderByRenderer(int material_index);
		virtual Matrix4x4 GetWorldMatrix();
		bool CheckBuffer(int material_index);
		void Render(int material_index, int pass_index);

	private:
		struct MaterialPass
		{
			int queue;
			int shader_pass_count;
			Renderer* renderer;
			int material_index;
			int shader_id;
			int material_id;
		};

		struct Passes
		{
			List<List<MaterialPass>> list;
			List<Renderer*> culled_renderers;
			bool passes_dirty;
			bool culling_dirty;

			Passes(): passes_dirty(true), culling_dirty(true) { }
		};

		struct RenderBuffer
		{
			const VertexBuffer* vb;
			const IndexBuffer* ib;
			int start;
			int count;

			RenderBuffer():
				vb(NULL),
				ib(NULL),
				start(0),
				count(0)
			{
			}
		};

		struct BatchInfo
		{
			int index_start;
			int index_count;
		};

		static void CheckPasses();
		static void CameraCulling();
		static void BuildPasses(const List<Renderer*>& renderers, List<List<MaterialPass>>& passes);
		static void BuildPasses();
		static void PreparePass(List<MaterialPass>& pass);
		static bool CheckPass(List<MaterialPass>& pass);
		static void CommitPass(List<MaterialPass>& pass);
		static void BindStaticBuffers();

		static List<Renderer*> m_renderers;
		static Map<Camera*, Passes> m_passes;
		static bool m_renderers_dirty;
		static Mutex m_mutex;
		static bool m_passes_dirty;
		static Ref<VertexBuffer> m_static_vertex_buffer;
		static Ref<IndexBuffer> m_static_index_buffer;
		static Vector<VertexAttributeOffset> m_static_vertex_attribute_offsets;
		static bool m_static_buffers_binding;
		static int m_batching_start;
		static int m_batching_count;

	protected:
		Vector<Ref<Material>> m_shared_materials;
		int m_sorting_order;
		int m_lightmap_index;
		Vector4 m_lightmap_scale_offset;
		Vector<RenderBuffer> m_buffer_old;
		Bounds m_bounds;
		Vector<BatchInfo> m_batch_indices;
	};
}
