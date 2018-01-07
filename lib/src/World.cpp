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

#include "World.h"
#include "Resource.h"
#include "Profiler.h"
#include "ui/Font.h"
#include "time/Time.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "graphics/RenderTexture.h"
#include "graphics/LightmapSettings.h"
#include "renderer/Renderer.h"
#include "audio/AudioManager.h"
#include "physics/Physics.h"
#include <stdlib.h>

namespace Viry3D
{
	FastList<Ref<GameObject>> World::m_gameobjects;
	List<Ref<GameObject>> World::m_gameobjects_start;
	Mutex World::m_mutex;

	void World::AddGameObject(const Ref<GameObject>& obj)
	{
		m_mutex.lock();
		m_gameobjects_start.AddLast(obj);
		m_mutex.unlock();
	}

	void World::AddGameObjects(const FastList<Ref<GameObject>>& objs)
	{
		m_mutex.lock();
		for (auto i = objs.Begin(); i != objs.End(); i = i->next)
		{
			m_gameobjects_start.AddLast(i->value);
		}
		m_mutex.unlock();
	}

	void World::Update()
	{
		Physics::Update();

		//	start
		int start_count = 0;
		do
		{
			List<Ref<GameObject>> starts;
			m_mutex.lock();
			starts = m_gameobjects_start;
			m_gameobjects_start.Clear();
			m_mutex.unlock();

			for (auto& i : starts)
			{
				auto& obj = i;
				if (!obj->m_deleted)
				{
					if (obj->IsActiveInHierarchy())
					{
						obj->Start();
					}

					if (!obj->m_in_world_update)
					{
						obj->m_in_world_update = true;
						m_gameobjects.AddLast(obj);
					}
				}
			}
			starts.Clear();

			m_mutex.lock();
			start_count = m_gameobjects_start.Size();
			m_mutex.unlock();
		} while (start_count > 0);

		//	update
		for (auto i = m_gameobjects.Begin(); i != m_gameobjects.End(); )
		{
			auto& obj = i->value;
			if (!obj->m_deleted)
			{
				if (obj->IsActiveInHierarchy())
				{
					obj->Start();
					obj->Update();
				}
			}
			else
			{
				i = m_gameobjects.Remove(i);
				continue;
			}

			i = i->next;
		}

		//	late update
		for (auto i = m_gameobjects.Begin(); i != m_gameobjects.End(); )
		{
			auto& obj = i->value;
			if (!obj->m_deleted)
			{
				if (obj->IsActiveInHierarchy())
				{
					obj->LateUpdate();
				}
			}
			else
			{
				i = m_gameobjects.Remove(i);
				continue;
			}

			i = i->next;
		}

		if (Renderer::IsRenderersDirty())
		{
			Renderer::SetRenderersDirty(false);
			Renderer::ClearPasses();

			List<Renderer*>& renderers = Renderer::GetRenderers();
			renderers.Clear();

			FindAllRenders(m_gameobjects, renderers, false, false, false);
		}
	}

	void World::FindAllRenders(const FastList<Ref<GameObject>>& objs, List<Renderer*>& renderers, bool include_inactive, bool include_disable, bool static_only)
	{
		auto i = objs.Begin();
		while (i != objs.End())
		{
			auto& obj = i->value;
			if (!obj->m_deleted)
			{
				if (include_inactive || obj->IsActiveInHierarchy())
				{
					if (!static_only || obj->IsStatic())
					{
						auto rs = obj->GetComponents<Renderer>();
						for (auto& i : rs)
						{
							if (include_disable || i->IsEnable())
							{
								renderers.AddLast(i.get());
							}
						}
					}
				}
			}

			i = i->next;
		}
	}

	void World::OnPause()
	{
		AudioManager::OnPause();
	}

	void World::OnResume()
	{
		AudioManager::OnResume();
	}

	void World::Init()
	{
		srand((unsigned int) Time::GetTimeMS());

		Component::RegisterComponents();

		Font::Init();
		Shader::Init();
		Object::Init();
		Camera::Init();
		RenderTexture::Init();
		AudioManager::Init();
		Renderer::Init();
		Physics::Init();
		Resource::Init();
	}

	void World::Deinit()
	{
		LightmapSettings::Clear();
		Resource::Deinit();
		m_gameobjects.Clear();
		m_gameobjects_start.Clear();
		Physics::Deinit();
		Renderer::Deinit();
		AudioManager::Deinit();
		RenderTexture::Deinit();
		Camera::Deinit();
		Object::Deinit();
		Shader::Deinit();
		Font::Deinit();
	}
}
