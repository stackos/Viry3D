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

#include "UICanvasRenderer.h"
#include "GameObject.h"
#include "UISprite.h"
#include "UILabel.h"
#include "Debug.h"
#include "graphics/Material.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(UICanvasRenderer);

	UICanvasRenderer::UICanvasRenderer():
		m_type(RenderType::BaseView)
	{
	}

	UICanvasRenderer::~UICanvasRenderer()
	{
	}

	bool UICanvasRenderer::IsRoot() const
	{
		return !GetParentRect();
	}

	void UICanvasRenderer::DeepCopy(const Ref<Object>& source)
	{
		Renderer::DeepCopy(source);

		auto src = RefCast<UICanvasRenderer>(source);
		this->SetSortingOrder(src->GetSortingOrder());
	}

	void UICanvasRenderer::HandleUIEvent(const List<UICanvasRenderer*>& list)
	{
		UIEventHandler::HandleUIEvent(list);
	}

	void UICanvasRenderer::LateUpdate()
	{
		UpdateViews();
	}

	void UICanvasRenderer::OnTranformHierarchyChanged()
	{
		MarkDirty();
	}

	void UICanvasRenderer::MarkDirty()
	{
		m_dirty = true;
	}

	void UICanvasRenderer::FindViews()
	{
		m_views.Clear();
		List<Ref<Transform>> to_find;
		to_find.AddFirst(this->GetTransform());

		while (!to_find.Empty())
		{
			auto t = to_find.First();
			to_find.RemoveFirst();

			auto view = t->GetGameObject()->GetComponent<UIView>();
			if (view &&
				view->IsEnable() &&
				t->GetGameObject()->IsActiveSelf())
			{
				auto type_name = view->GetTypeName();
				if (type_name == "UISprite")
				{
					if (m_type == RenderType::BaseView || m_type == RenderType::Sprite)
					{
						m_type = RenderType::Sprite;
						m_views.Add(view);
					}
				}
				else if (type_name == "UILabel")
				{
					if (m_type == RenderType::BaseView || m_type == RenderType::Text)
					{
						m_type = RenderType::Text;
						m_views.Add(view);
					}
				}
				else if (type_name == "UIView")
				{
					// do not render empty view
				}
				else
				{
					assert(!"unknown view type");
				}
			}

			int child_count = t->GetChildCount();
			for (int i = child_count - 1; i >= 0; i--)
			{
				auto child = t->GetChild(i);
				auto canvas = child->GetGameObject()->GetComponent<UICanvasRenderer>();
				if (!canvas && child->GetGameObject()->IsActiveSelf())
				{
					to_find.AddFirst(child);
				}
			}
		}

		for (int i = 0; i < m_views.Size(); i++)
		{
			m_views[i]->SetRenderer(RefCast<UICanvasRenderer>(this->GetRef()));
		}
	}

	void UICanvasRenderer::UpdateViews()
	{
		if (!m_dirty)
		{
			return;
		}

		m_dirty = false;

		FindViews();

		if (!m_views.Empty())
		{
			auto mat = this->GetSharedMaterial();
			if (!mat)
			{
				if (m_type == RenderType::BaseView || m_type == RenderType::Sprite)
				{
					mat = Material::Create("UI/Sprite");
					this->SetSharedMaterial(mat);
				}
				else if (m_type == RenderType::Text)
				{
					mat = Material::Create("UI/Text");
					this->SetSharedMaterial(mat);
				}
			}

			Vector<Vector3> vertices;
			Vector<Vector2> uv;
			Vector<Color> colors;
			Vector<unsigned short> indices;

			for (int i = 0; i < m_views.Size(); i++)
			{
				m_views[i]->SetRenderer(RefCast<UICanvasRenderer>(this->GetRef()));
				m_views[i]->FillVertices(vertices, uv, colors, indices);
				m_views[i]->FillMaterial(mat);
			}

			if (!vertices.Empty())
			{
				if (!m_mesh)
				{
					m_mesh = Mesh::Create(true);
				}
				m_mesh->vertices = vertices;
				m_mesh->uv = uv;
				m_mesh->colors = colors;
				m_mesh->triangles = indices;
				m_mesh->Update();
			}
		}
	}

	const VertexBuffer* UICanvasRenderer::GetVertexBuffer() const
	{
		if (m_mesh)
		{
			return m_mesh->GetVertexBuffer().get();
		}

		return NULL;
	}

	const IndexBuffer* UICanvasRenderer::GetIndexBuffer() const
	{
		if (m_mesh)
		{
			return m_mesh->GetIndexBuffer().get();
		}

		return NULL;
	}

	void UICanvasRenderer::GetIndexRange(int material_index, int& start, int& count) const
	{
		if (m_mesh)
		{
			m_mesh->GetIndexRange(material_index, start, count);
		}
	}
}
