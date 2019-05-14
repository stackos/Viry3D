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

#include "GameObject.h"
#include "Scene.h"

namespace Viry3D
{
	Ref<GameObject> GameObject::Create(const String& name)
	{
		Ref<GameObject> obj = Ref<GameObject>(new GameObject(name));
		Scene::Instance()->AddGameObject(obj);
        obj->m_transform = obj->AddComponent<Transform>();
		return obj;
	}
	
	void GameObject::Destroy(Ref<GameObject>& obj)
	{
		Scene::Instance()->RemoveGameObject(obj);
		obj.reset();
	}

	GameObject::GameObject(const String& name):
        m_layer(0),
		m_is_active_self(true),
		m_is_active_in_tree(true)
	{
		this->SetName(name);
	}
    
	GameObject::~GameObject()
    {
        m_added_components.Clear();
        m_removed_components.Clear();
        m_components.Clear();
        m_transform.reset();
    }
    
    void GameObject::RemoveComponent(const Ref<Component>& com)
    {
        if (com == m_transform)
        {
            return;
        }
        
        if (!m_added_components.Remove(com))
        {
            m_removed_components.Add(com);
        }
    }
    
    void GameObject::BindComponent(const Ref<Component>& com) const
    {
        auto obj = Scene::Instance()->GetGameObject(this);
        com->m_object = obj;
		com->SetName(this->GetName());
    }

	void GameObject::OnTransformDirty()
	{
		for (int i = 0; i < m_added_components.Size(); ++i)
		{
			auto& com = m_added_components[i];
			com->OnTransformDirty();
		}

		for (int i = 0; i < m_components.Size(); ++i)
		{
			auto& com = m_components[i];
			com->OnTransformDirty();
		}
	}
    
	void GameObject::SetLayer(int layer)
	{
		m_layer = layer;
	}

	void GameObject::SetActive(bool active)
	{
		m_is_active_self = active;
	}

	void GameObject::Update()
	{
        for (int i = 0; i < m_components.Size(); ++i)
        {
            auto& com = m_components[i];
            com->Update();
        }
        
        do
        {
            Vector<Ref<Component>> added = m_added_components;
            m_added_components.Clear();
            
            for (int i = 0; i < added.Size(); ++i)
            {
                auto& com = added[i];
                com->Update();
                m_components.Add(com);
            }
            added.Clear();
        } while (m_added_components.Size() > 0);
        
        for (int i = 0; i < m_removed_components.Size(); ++i)
        {
            auto& com = m_removed_components[i];
            m_components.Remove(com);
        }
        m_removed_components.Clear();
	}
}
