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

#include "UIRect.h"
#include "renderer/Renderer.h"

namespace Viry3D
{
	class Camera;
	class Mesh;
	class UIView;

	class UICanvasRenderer: public Renderer, public UIRect
	{
		DECLARE_COM_CLASS(UICanvasRenderer, Renderer);

	private:
		friend class UIEventHandler;

	public:
		enum class RenderType
		{
			BaseView,
			Sprite,
			Text
		};

		static void HandleUIEvent(const List<UICanvasRenderer*>& list);

		virtual ~UICanvasRenderer();
		virtual const VertexBuffer* GetVertexBuffer();
		virtual const IndexBuffer* GetIndexBuffer();
		virtual void GetIndexRange(int material_index, int& start, int& count);
		void MarkDirty();
		const Vector<Ref<UIView>>& GetViews() const { return m_views; }

	protected:
		virtual void LateUpdate();
		virtual void OnTranformHierarchyChanged();

	private:
		UICanvasRenderer();
		void FindViews();
		void UpdateViews();

		RenderType m_type;
		Ref<Mesh> m_mesh;
		Vector<Ref<UIView>> m_views;
	};
}
