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

#include "Object.h"
#include "Component.h"
#include "Transform.h"
#include "container/Vector.h"
#include "container/List.h"

namespace Viry3D
{
	class GameObject: public Object
	{
		friend class World;
		friend class Transform;
		friend class Renderer;
		friend class Camera;

	public:
		//
		//	说明
		//	普通情况下, 对象在创建时自动加入World,
		//	在子线程中创建对象时, 可以暂时不把对象添加到World, 待子对象和组件都创建完成后, 统一手动添加
		//
		static Ref<GameObject> Create(const String& name, bool add_to_world = true);
		static void Destroy(const Ref<GameObject>& obj);
		static Ref<GameObject> Instantiate(const Ref<GameObject>& source);

		virtual ~GameObject();
		void DeepCopy(const Ref<Object>& source);
		Ref<Component> AddComponent(const String& name);
		Ref<Component> GetComponent(const String& name) const;
		Vector<Ref<Component>> GetComponentsInChildren(const String& name) const;
		template<class T> Ref<T> AddComponent();
		template<class T> Ref<T> GetComponent() const;
		template<class T> Vector<Ref<T>> GetComponents() const;
		template<class T> Vector<Ref<T>> GetComponentsInChildren() const;
		template<class T> Ref<T> GetComponentInParent() const;
		Ref<Component> GetComponentRef(const Component* com) const;
		Ref<Transform> GetTransform() const { return m_transform.lock(); }
		bool IsActiveInHierarchy() const { return m_active_in_hierarchy; }
		bool IsActiveSelf() const { return m_active_self; }
		void SetActive(bool active);
		void SetLayerRecursively(int layer);
		bool IsStatic() const { return m_static; }
		void SetStatic(bool value) { m_static = value; }
		void SetName(const String& name);
		void SetLayer(int layer);
		int GetLayer() const { return m_layer; }

	private:
		GameObject(const String& name);
		void Delete();
		void Start();
		void Update();
		void LateUpdate();
		void AddComponent(const Ref<Component>& com);
		void SetActiveInHierarchy(bool active);
		void CopyComponent(const Ref<Component>& com);
		void OnTranformChanged();
		void OnTranformHierarchyChanged();
		void OnPostRender(); // 仅带摄像机的对象

		int m_layer;
		List<Ref<Component>> m_components;
		List<Ref<Component>> m_components_new;
		bool m_active_in_hierarchy;
		bool m_active_self;
		bool m_deleted;
		WeakRef<Transform> m_transform;
		bool m_static;
		bool m_in_world;
		bool m_in_world_update;
	};

	template<class T> Ref<T> GameObject::AddComponent()
	{
		return RefCast<T>(AddComponent(T::ClassName()));
	}

	template<class T> Ref<T> GameObject::GetComponent() const
	{
		for (auto i : m_components)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				return t;
			}
		}

		for (auto i : m_components_new)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				return t;
			}
		}

		return Ref<T>();
	}

	template<class T> Vector<Ref<T>> GameObject::GetComponents() const
	{
		Vector<Ref<T>> coms;

		for (auto i : m_components)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				coms.Add(t);
			}
		}

		for (auto i : m_components_new)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				coms.Add(t);
			}
		}

		return coms;
	}

	template<class T> Vector<Ref<T>> GameObject::GetComponentsInChildren() const
	{
		Vector<Ref<T>> coms;

		for (auto i : m_components)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				coms.Add(t);
			}
		}

		for (auto i : m_components_new)
		{
			auto t = RefCast<T>(i);
			if (t && !i->m_deleted)
			{
				coms.Add(t);
			}
		}

		auto transform = GetTransform();
		int child_count = transform->GetChildCount();
		for (int i = 0; i < child_count; i++)
		{
			auto child = transform->GetChild(i);
			auto child_coms = child->GetGameObject()->GetComponentsInChildren<T>();

			if (!child_coms.Empty())
			{
				coms.AddRange(&child_coms[0], child_coms.Size());
			}
		}

		return coms;
	}

	template<class T> Ref<T> GameObject::GetComponentInParent() const
	{
		Ref<T> com;

		auto parent = GetTransform()->GetParent().lock();

		while (parent)
		{
			com = parent->GetGameObject()->GetComponent<T>();

			if (com)
			{
				break;
			}
			else
			{
				parent = parent->GetParent().lock();
			}
		}

		return com;
	}
}
