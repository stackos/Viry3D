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
#include "GameObject.h"
#include "UIRect.h"
#include "UIEventHandler.h"
#include "graphics/Color.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
	class UICanvasRenderer;
	class Material;

	class UIView: public Component, public UIRect
	{
		DECLARE_COM_CLASS(UIView, Component);

	public:
		virtual void SetAnchors(const Vector2& min, const Vector2& max);
		virtual void SetOffsets(const Vector2& min, const Vector2& max);
		virtual void SetPivot(const Vector2& pivot);
		virtual void FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices);
		virtual void FillMaterial(Ref<Material>& mat);
		void SetColor(const Color& color);
		const Color& GetColor() const { return m_color; }
		void SetRenderer(const Ref<UICanvasRenderer>& renderer);
		const WeakRef<UICanvasRenderer>& GetRenderer() const { return m_renderer; }
		Vector<Vector3> GetBoundsVertices();

	protected:
		UIView();
		void MarkRendererDirty();
		Matrix4x4 GetVertexMatrix();
		virtual void OnTranformChanged();

	public:
		UIEventHandler event_handler;

	protected:
		Color m_color;
		WeakRef<UICanvasRenderer> m_renderer;
	};
}
