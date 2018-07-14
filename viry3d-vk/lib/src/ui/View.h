/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "container/Vector.h"
#include "graphics/Color.h"
#include "graphics/VertexAttribute.h"
#include "math/Vector2.h"
#include "math/Vector2i.h"
#include "math/Quaternion.h"
#include "math/Rect.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
	class CanvaRenderer;
	class Texture;
    class View;

    struct ViewMesh
    {
        Vector<Vertex> vertices;
        Vector<unsigned short> indices;
        Ref<Texture> texture;
        View* view = nullptr;
        bool base_view = false;
    };

    struct ViewAlignment
	{
        enum
        {
            Left = 0x00000001,
            HCenter = 0x00000002,
            Right = 0x00000004,
            Top = 0x00000010,
            VCenter = 0x00000020,
            Bottom = 0x00000040,
        };
	};

	class View
	{
	public:
		View();
		virtual ~View();
        virtual void UpdateLayout();
		void OnAddToCanvas(CanvaRenderer* canvas);
		void OnRemoveFromCanvas(CanvaRenderer* canvas);
        CanvaRenderer* GetCanvas() const { return m_canvas; }
        void AddSubview(const Ref<View>& view);
        void RemoveSubview(const Ref<View>& view);
        int GetSubviewCount() const { return m_subviews.Size(); }
        const Ref<View>& GetSubview(int index) const { return m_subviews[index]; }
		const Color& GetColor() const { return m_color; }
		void SetColor(const Color& color);
		int GetAlignment() const { return m_alignment; }
        // use ViewAlignment
		void SetAlignment(int alignment);
		const Vector2& GetPivot() const { return m_pivot; }
        // left top is (0.0, 0.0), right bottom is (1.0, 1.0)
		void SetPivot(const Vector2& pivot);
		const Vector2i& GetSize() const { return m_size; }
		void SetSize(const Vector2i& size);
		const Vector2i& GetOffset() const { return m_offset; }
        // offset y direction is down
		void SetOffset(const Vector2i& offset);
        const Quaternion& GetLocalRotation() const { return m_local_rotation; }
        void SetLocalRotation(const Quaternion& rotation);
        const Vector2& GetLocalScale() const { return m_local_scale; }
        void SetLocalScale(const Vector2& scale);
        const Rect& GetRect() const { return m_rect; }
        const Quaternion& GetRotation() const { return m_rotation; }
        const Vector2& GetScale() const { return m_scale; }
        void FillMeshes(Vector<ViewMesh>& mesh);

    protected:
        virtual void FillSelfMeshes(Vector<ViewMesh>& meshes);
        void ComputeVerticesRectAndMatrix(Rect& rect, Matrix4x4& matrix);

	private:
		CanvaRenderer* m_canvas;
        View* m_parent_view;
		Vector<Ref<View>> m_subviews;
		Color m_color;
		int m_alignment;
		Vector2 m_pivot;
        Vector2i m_size;
        Vector2i m_offset;
        Quaternion m_local_rotation;
        Vector2 m_local_scale;
        Rect m_rect;
        Quaternion m_rotation;
        Vector2 m_scale;
	};
}
