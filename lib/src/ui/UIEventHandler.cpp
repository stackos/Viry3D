#include "UIEventHandler.h"
#include "UICanvasRenderer.h"
#include "UIView.h"
#include "Input.h"
#include "graphics/Graphics.h"

namespace Viry3D
{
	static Map<int, Vector<WeakRef<UIView>>> g_hit_views;

	static Vector<WeakRef<UIView>> hit_test(Vector2 position, const Vector<Ref<UIView>>& views)
	{
		Vector<WeakRef<UIView>> hit_views;

		Vector2 pos_world;
		pos_world.x = position.x - Graphics::GetDisplay()->GetWidth() / 2;
		pos_world.y = position.y - Graphics::GetDisplay()->GetHeight() / 2;

		for(auto& i : views)
		{
			auto mat = i->GetRenderer().lock()->GetTransform()->GetLocalToWorldMatrix();
			auto vertices = i->GetBoundsVertices();
			for(int j = 0; j < vertices.Size(); j++)
			{
				// from canvas space to world space
				vertices[j] = mat.MultiplyPoint3x4(vertices[j]);
			}

			if(pos_world.x > vertices[0].x &&
				pos_world.x < vertices[1].x &&
				pos_world.y > vertices[1].y &&
				pos_world.y < vertices[2].y)
			{
				hit_views.Add(i);
			}
		}
		
		return hit_views;
	}

	void UIEventHandler::HandleUIEvent(const List<UICanvasRenderer*>& list)
	{
		int touch_count = Input::GetTouchCount();
		if(touch_count == 0)
		{
			return;
		}

		Vector<Ref<UIView>> views;
		for(auto i : list)
		{
			i->FindViews();
			auto& vs = i->GetViews();

			for(auto& j : vs)
			{
				views.Add(j);
			}
		}

		for(int i = 0; i < touch_count; i++)
		{
			auto t = Input::GetTouch(i);

			UIPointerEvent e;
			e.position = t->position;

			if(t->phase == TouchPhase::Began)
			{
				if(!g_hit_views.Contains(t->fingerId))
				{
					g_hit_views.Add(t->fingerId, Vector<WeakRef<UIView>>());
				}
				auto& pointer_views = g_hit_views[t->fingerId];

				auto hit_views = hit_test(t->position, views);
				for(int j = hit_views.Size() - 1; j >= 0; j--)
				{
					auto event_handler = hit_views[j].lock()->event_handler;
					if(event_handler.enable)
					{
						// send down event to top view in down
						auto on_pointer_down = event_handler.on_pointer_down;
						if(on_pointer_down)
						{
							on_pointer_down(e);
						}

						pointer_views.Add(hit_views[j]);
					}
				}
			}
			else if(t->phase == TouchPhase::Moved)
			{
				//auto& pointer_views = g_hit_views[t->fingerId];
			}
			else if(t->phase == TouchPhase::Ended || t->phase == TouchPhase::Canceled)
			{
				auto& pointer_views = g_hit_views[t->fingerId];

				auto hit_views = hit_test(t->position, views);
				for(int j = hit_views.Size() - 1; j >= 0; j--)
				{
					auto v = hit_views[j].lock();

					if(v->event_handler.enable)
					{
						// send up event to top view in up
						auto on_pointer_up = v->event_handler.on_pointer_up;
						if(on_pointer_up)
						{
							on_pointer_up(e);
						}

						// send click event to top view in down and in up
						for(auto& k : pointer_views)
						{
							if(!k.expired() && k.lock() == v)
							{
								auto on_pointer_click = v->event_handler.on_pointer_click;
								if(on_pointer_click)
								{
									on_pointer_click(e);
								}
								break;
							}
						}

						break;
					}
				}

				// send up event to top view in down but not in up
				for(auto& j : pointer_views)
				{
					auto v = j.lock();
					bool not_hit = true;

					for(auto& k : hit_views)
					{
						if(v == k.lock())
						{
							not_hit = false;
							break;
						}
					}

					if(not_hit)
					{
						if(v->event_handler.enable)
						{
							auto on_pointer_up = v->event_handler.on_pointer_up;
							if(on_pointer_up)
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