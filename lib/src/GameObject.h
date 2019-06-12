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

#pragma once

#include "Object.h"
#include "container/Vector.h"
#include "Component.h"
#include "Transform.h"

namespace Viry3D
{
    class GameObject : public Object
    {
    public:
		static Ref<GameObject> Create(const String& name);
		static void Destroy(Ref<GameObject>& obj);
        virtual ~GameObject();
        template <class T, typename ...ARGS> Ref<T> AddComponent(ARGS... args);
        template <class T> Ref<T> GetComponent() const;
		template <class T> Vector<Ref<T>> GetComponentsInChildren() const;
        void RemoveComponent(const Ref<Component>& com);
        const Ref<Transform>& GetTransform() const { return m_transform; }
        int GetLayer() const { return m_layer; }
		void SetLayer(int layer);
		bool IsActiveSelf() const { return m_is_active_self; }
		void SetActive(bool active);
		bool IsActiveInTree() const { return m_is_active_in_tree; }
		void Update();
		
	private:
		GameObject(const String& name);
        void BindComponent(const Ref<Component>& com) const;
		void OnTransformDirty();
	
	private:
		friend class Transform;

	private:
        Vector<Ref<Component>> m_components;
        Vector<Ref<Component>> m_added_components;
        Vector<Ref<Component>> m_removed_components;
        Ref<Transform> m_transform;
        int m_layer;
        bool m_is_active_self;
        bool m_is_active_in_tree;
    };
    
    template <class T, typename ...ARGS>
    Ref<T> GameObject::AddComponent(ARGS... args)
    {
        Ref<T> com = RefMake<T>(args...);
        
        auto is_transform = RefCast<Transform>(com);
        if (m_transform && is_transform)
        {
            return Ref<T>();
        }
        
        m_added_components.Add(com);
        
        this->BindComponent(com);
        
        return com;
    }
    
    template <class T>
    Ref<T> GameObject::GetComponent() const
    {
        for (int i = 0; i < m_added_components.Size(); ++i)
        {
            auto& com = m_added_components[i];
            auto t = RefCast<T>(com);
            if (t)
            {
                return t;
            }
        }
        
        for (int i = 0; i < m_components.Size(); ++i)
        {
            auto& com = m_components[i];
            auto t = RefCast<T>(com);
            if (t)
            {
                return t;
            }
        }
        
        return Ref<T>();
    }

	template <class T>
	Vector<Ref<T>> GameObject::GetComponentsInChildren() const
	{
		Vector<Ref<T>> coms;

		for (int i = 0; i < m_added_components.Size(); ++i)
		{
			auto& com = m_added_components[i];
			auto t = RefCast<T>(com);
			if (t)
			{
				coms.Add(t);
			}
		}

		for (int i = 0; i < m_components.Size(); ++i)
		{
			auto& com = m_components[i];
			auto t = RefCast<T>(com);
			if (t)
			{
				coms.Add(t);
			}
		}

		int child_count = this->GetTransform()->GetChildCount();
		for (int i = 0; i < child_count; ++i)
		{
			auto child = this->GetTransform()->GetChild(i);
			Vector<Ref<T>> child_coms = child->GetGameObject()->GetComponentsInChildren<T>();
			for (int j = 0; j < child_coms.Size(); ++j)
			{
				coms.Add(child_coms[j]);
			}
		}

		return coms;
	}
}
