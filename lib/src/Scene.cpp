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

#include "Scene.h"
#include "GameObject.h"
#include "App.h"

namespace Viry3D
{
	Scene* Scene::m_instance = nullptr;

	Scene* Scene::Instance()
	{
		return m_instance;
	}

    Scene::Scene()
    {
		m_instance = this;

		GameObject::Create("App")->AddComponent<App>();
    }
    
    Scene::~Scene()
    {
		m_added_objects.Clear();
		m_removed_objects.Clear();
		m_objects.Clear();

		m_instance = nullptr;
    }

	void Scene::AddGameObject(const Ref<GameObject>& obj)
	{
		m_added_objects.Add(obj);
	}

	void Scene::RemoveGameObject(const Ref<GameObject>& obj)
	{
        if (!m_added_objects.Remove(obj))
        {
            m_removed_objects.Add(obj);
        }
	}
    
    void Scene::Update()
    {
		for (auto& i : m_objects)
		{
			auto& obj = i.second;
			if (obj->IsActiveInTree())
			{
				obj->Update();
			}
		}

		do
		{
			Vector<Ref<GameObject>> added = m_added_objects;
			m_added_objects.Clear();

			for (int i = 0; i < added.Size(); ++i)
			{
				auto& obj = added[i];
				if (obj->IsActiveInTree())
				{
					obj->Update();
				}
				m_objects.Add(obj->GetId(), obj);
			}
			added.Clear();
		} while (m_added_objects.Size() > 0);

		for (int i = 0; i < m_removed_objects.Size(); ++i)
		{
			const auto& obj = m_removed_objects[i];
			m_objects.Remove(obj->GetId());
		}
		m_removed_objects.Clear();
    }
    
    Ref<GameObject> Scene::GetGameObject(const GameObject* obj)
    {
        for (int i = 0; i < m_added_objects.Size(); ++i)
        {
            const auto& obj_ref = m_added_objects[i];
            if (obj_ref.get() == obj)
            {
                return obj_ref;
            }
        }
        
        Ref<GameObject>* find;
        if (m_objects.TryGet(obj->GetId(), &find))
        {
            return *find;
        }
        
        return Ref<GameObject>();
    }
}
