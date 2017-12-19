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

#include "UIEventHandler.h"
#include "UICanvasRenderer.h"
#include "UIView.h"
#include "Input.h"
#include "graphics/Graphics.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	static Map<int, Vector<WeakRef<UIView>>> g_hit_views;
	bool UIEventHandler::m_has_event;

	static Vector<WeakRef<UIView>> hit_test(const Vector2& position, const Vector<Ref<UIView>>& views)
	{
		Vector<WeakRef<UIView>> hit_views;

		if (views.Size() > 0)
		{
			auto camera = views[0]->GetRenderer().lock()->GetRootCanvas()->GetCamera();

			// only handle default frame buffer
			if (camera->GetFrameBuffer())
			{
				return hit_views;
			}

			for (auto& i : views)
			{
				auto mat = i->GetRenderer().lock()->GetTransform()->GetLocalToWorldMatrix();
				Vector<Vector3> vertices;
				i->GetBoundsVertices(vertices);
				for (int j = 0; j < vertices.Size(); j++)
				{
					// from canvas space to world space
					vertices[j] = mat.MultiplyPoint3x4(vertices[j]);
					// to screen space
					vertices[j] = camera->WorldToScreenPoint(vertices[j]);
				}

				if (position.x > vertices[0].x &&
					position.x < vertices[1].x &&
					position.y > vertices[1].y &&
					position.y < vertices[2].y)
				{
					hit_views.Add(i);
				}
			}
		}

		return hit_views;
	}

	void UIEventHandler::HandleUIEvent(const List<UICanvasRenderer*>& list)
	{
		m_has_event = false;

		int touch_count = Input::GetTouchCount();
		if (touch_count == 0)
		{
			return;
		}

		Vector<Ref<UIView>> views;
		for (auto i : list)
		{
			i->FindViews();
			auto& vs = i->GetViews();

			for (auto& j : vs)
			{
				views.Add(j);
			}
		}

		for (int i = 0; i < touch_count; i++)
		{
			auto t = Input::GetTouch(i);

			UIPointerEvent e;
			e.position = t->position;

			if (t->phase == TouchPhase::Began)
			{
				if (!g_hit_views.Contains(t->fingerId))
				{
					g_hit_views.Add(t->fingerId, Vector<WeakRef<UIView>>());
				}
				auto& pointer_views = g_hit_views[t->fingerId];

				auto hit_views = hit_test(t->position, views);
				for (int j = hit_views.Size() - 1; j >= 0; j--)
				{
					auto event_handler = hit_views[j].lock()->event_handler;
					if (event_handler.enable)
					{
						// send down event to top view in down
						auto on_pointer_down = event_handler.on_pointer_down;
						if (on_pointer_down)
						{
							on_pointer_down(e);
						}

						pointer_views.Add(hit_views[j]);

						m_has_event = true;
					}
				}
			}
			else if (t->phase == TouchPhase::Moved)
			{
				//auto& pointer_views = g_hit_views[t->fingerId];
			}
			else if (t->phase == TouchPhase::Ended || t->phase == TouchPhase::Canceled)
			{
				auto& pointer_views = g_hit_views[t->fingerId];

				auto hit_views = hit_test(t->position, views);
				for (int j = hit_views.Size() - 1; j >= 0; j--)
				{
					auto v = hit_views[j].lock();

					if (v->event_handler.enable)
					{
						// send up event to top view in up
						auto on_pointer_up = v->event_handler.on_pointer_up;
						if (on_pointer_up)
						{
							on_pointer_up(e);
						}

						// send click event to top view in down and in up
						for (auto& k : pointer_views)
						{
							if (!k.expired() && k.lock() == v)
							{
								auto on_pointer_click = v->event_handler.on_pointer_click;
								if (on_pointer_click)
								{
									on_pointer_click(e);
								}
								break;
							}
						}

						m_has_event = true;
						break;
					}
				}

				// send up event to top view in down but not in up
				for (auto& j : pointer_views)
				{
					auto v = j.lock();
					bool not_hit = true;

					for (auto& k : hit_views)
					{
						if (v == k.lock())
						{
							not_hit = false;
							break;
						}
					}

					if (not_hit)
					{
						if (v->event_handler.enable)
						{
							auto on_pointer_up = v->event_handler.on_pointer_up;
							if (on_pointer_up)
							{
								on_pointer_up(e);
							}

							break;
						}
					}
				}

				g_hit_views.Remove(t->fingerId);
			}
		}
	}
}
