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

#include "GameObject.h"
#include "Layer.h"
#include "World.h"
#include "renderer/Renderer.h"

namespace Viry3D
{
	Ref<GameObject> GameObject::Create(const String& name, bool add_to_world)
	{
		auto obj = Ref<GameObject>(new GameObject(name));

		if (add_to_world)
		{
			obj->m_in_world = true;
			World::AddGameObject(obj);
		}

		auto transform = Ref<Transform>((Transform*) Component::Create(Transform::ClassName()));
		transform->m_gameobject = obj;
		obj->m_transform = transform;
		obj->AddComponent(transform);

		return obj;
	}

	void GameObject::Destroy(const Ref<GameObject>& obj)
	{
		if (obj)
		{
			obj->GetTransform()->SetParent(WeakRef<Transform>());
			obj->SetActive(false);
			obj->Delete();
		}
	}

	void GameObject::CopyComponent(const Ref<Component>& com)
	{
		auto transform = RefCast<Transform>(com);

		if (transform)
		{
			GetTransform()->DeepCopy(transform);
		}
		else
		{
			auto class_name = com->GetTypeName();
			auto* p_com = Component::Create(class_name);
			if (p_com != NULL)
			{
				AddComponent(Ref<Component>(p_com));
				p_com->DeepCopy(com);
			}
		}
	}

	void GameObject::DeepCopy(const Ref<Object>& source)
	{
		Object::DeepCopy(source);

		auto src = RefCast<GameObject>(source);

		for (const auto& i : src->m_components)
		{
			CopyComponent(i);
		}

		for (const auto& i : src->m_components_new)
		{
			CopyComponent(i);
		}

		this->SetLayer(src->GetLayer());
		m_active_in_hierarchy = src->m_active_in_hierarchy;
		m_active_self = src->m_active_self;
		m_deleted = src->m_deleted;
		m_static = src->m_static;
	}

	void GameObject::SetName(const String& name)
	{
		if (GetName() != name)
		{
			Object::SetName(name);

			for (const auto& i : m_components)
			{
				i->SetName(name);
			}

			for (const auto& i : m_components_new)
			{
				i->SetName(name);
			}
		}
	}

	Ref<GameObject> GameObject::Instantiate(const Ref<GameObject>& source)
	{
		Ref<GameObject> clone = GameObject::Create(source->GetName());

		clone->DeepCopy(RefCast<Object>(source));

		return clone;
	}

	GameObject::GameObject(const String& name):
		m_layer((int) Layer::Default),
		m_active_in_hierarchy(true),
		m_active_self(true),
		m_deleted(false),
		m_static(false),
		m_in_world(false),
		m_in_world_update(false)
	{
		this->SetName(name);
	}

	GameObject::~GameObject()
	{
	}

	void GameObject::Delete()
	{
		if (!m_deleted)
		{
			m_deleted = true;
		}

		auto transform = GetTransform();
		int child_count = transform->GetChildCount();
		for (int i = 0; i < child_count; i++)
		{
			transform->GetChild(i)->GetGameObject()->Delete();
		}
	}

	void GameObject::SetLayer(int layer)
	{
		if (m_layer != layer)
		{
			m_layer = layer;

			auto renderer = GetComponent<Renderer>();
			if (renderer)
			{
				Renderer::ClearPasses();
			}

			this->OnLayerChanged();
		}
	}

	void GameObject::SetLayerRecursively(int layer)
	{
		this->SetLayer(layer);

		auto transform = m_transform.lock();
		int child_count = transform->GetChildCount();
		for (int i = 0; i < child_count; i++)
		{
			auto child = transform->GetChild(i);
			child->GetGameObject()->SetLayerRecursively(layer);
		}
	}

	void GameObject::Start()
	{
		List<Ref<Component>> starts(m_components);
		do
		{
			for (auto& i : starts)
			{
				if (!IsActiveInHierarchy() || m_deleted)
				{
					break;
				}

				if (i->IsEnable() && !i->IsStarted())
				{
					i->m_started = true;
					i->Start();
				}
			}
			starts.Clear();

			starts = m_components_new;
			m_components.AddRangeBefore(m_components.end(), m_components_new.begin(), m_components_new.end());
			m_components_new.Clear();
		} while (!starts.Empty());
	}

	void GameObject::Update()
	{
		for (const auto& i : m_components)
		{
			if (!IsActiveInHierarchy() || m_deleted)
			{
				break;
			}

			if (i->IsEnable())
			{
				i->Update();
			}
		}
	}

	void GameObject::LateUpdate()
	{
		for (const auto& i : m_components)
		{
			if (!IsActiveInHierarchy() || m_deleted)
			{
				break;
			}

			if (i->IsEnable())
			{
				i->LateUpdate();
			}
		}

		//delete component
		auto it = m_components.begin();
		while (it != m_components.end())
		{
			if ((*it)->m_deleted)
			{
				it = m_components.Remove(it);
			}
			else
			{
				it++;
			}
		}
	}

	Ref<Component> GameObject::AddComponent(const String& name)
	{
		if (m_deleted)
		{
			return Ref<Component>();
		}

		auto t = Ref<Component>(Component::Create(name));
		AddComponent(t);

		return t;
	}

	Ref<Component> GameObject::GetComponent(const String& name) const
	{
		for (auto i : m_components)
		{
			if (!i->m_deleted && i->IsComponent(name))
			{
				return i;
			}
		}

		for (auto i : m_components_new)
		{
			if (!i->m_deleted && i->IsComponent(name))
			{
				return i;
			}
		}

		return Ref<Component>();
	}

	Vector<Ref<Component>> GameObject::GetComponentsInChildren(const String& name) const
	{
		Vector<Ref<Component>> coms;

		for (auto i : m_components)
		{
			if (!i->m_deleted && i->IsComponent(name))
			{
				coms.Add(i);
			}
		}

		for (auto i : m_components_new)
		{
			if (!i->m_deleted && i->IsComponent(name))
			{
				coms.Add(i);
			}
		}

		auto transform = GetTransform();
		int child_count = transform->GetChildCount();
		for (int i = 0; i < child_count; i++)
		{
			auto child = transform->GetChild(i);
			auto child_coms = child->GetGameObject()->GetComponentsInChildren(name);

			if (!child_coms.Empty())
			{
				coms.AddRange(&child_coms[0], child_coms.Size());
			}
		}

		return coms;
	}

	void GameObject::AddComponent(const Ref<Component>& com)
	{
		m_components_new.AddLast(com);

		com->m_transform = m_transform;
		com->m_gameobject = m_transform.lock()->m_gameobject;
		com->SetName(GetName());
		com->Awake();

		if (m_in_world)
		{
			World::AddGameObject(this->GetTransform()->GetGameObject());
		}
	}

	void GameObject::SetActive(bool active)
	{
		if (m_active_self != active)
		{
			m_active_self = active;

			auto t = m_transform.lock();
			if (m_active_in_hierarchy != active &&
				(t->IsRoot() || t->GetParent().lock()->GetGameObject()->IsActiveInHierarchy()))
			{
				SetActiveInHierarchy(active);

				if (!t->IsRoot())
				{
					t->GetParent().lock()->NotifyParentHierarchyChange();
				}
				t->NotifyChildHierarchyChange();
			}

			if (m_active_self && m_in_world)
			{
				World::AddGameObject(this->GetTransform()->GetGameObject());
			}

			auto renderer = GetComponent<Renderer>();
			if (renderer)
			{
				Renderer::SetRenderersDirty(true);
			}
		}
	}

	void GameObject::SetActiveInHierarchy(bool active)
	{
		if (m_active_in_hierarchy != active)
		{
			m_active_in_hierarchy = active;

			for (const auto& i : m_components)
			{
				if (i->IsEnable())
				{
					if (m_active_in_hierarchy)
					{
						i->OnEnable();
					}
					else
					{
						i->OnDisable();
					}
				}
			}

			auto transform = m_transform.lock();
			int child_count = transform->GetChildCount();
			for (int i = 0; i < child_count; i++)
			{
				auto child = transform->GetChild(i);
				if (child->GetGameObject()->IsActiveSelf())
				{
					child->GetGameObject()->SetActiveInHierarchy(active);
				}
			}
		}
	}

	Ref<Component> GameObject::GetComponentRef(const Component* com) const
	{
		for (const auto& i : m_components)
		{
			if (i.get() == com && !i->m_deleted)
			{
				return i;
			}
		}

		for (const auto& i : m_components_new)
		{
			if (i.get() == com && !i->m_deleted)
			{
				return i;
			}
		}

		return Ref<Component>();
	}

	void GameObject::OnTranformChanged()
	{
		for (const auto& i : m_components)
		{
			i->OnTranformChanged();
		}

		for (const auto& i : m_components_new)
		{
			i->OnTranformChanged();
		}
	}

	void GameObject::OnTranformHierarchyChanged()
	{
		for (const auto& i : m_components)
		{
			i->OnTranformHierarchyChanged();
		}

		for (const auto& i : m_components_new)
		{
			i->OnTranformHierarchyChanged();
		}
	}

	void GameObject::OnLayerChanged()
	{
		for (const auto& i : m_components)
		{
			i->OnLayerChanged();
		}

		for (const auto& i : m_components_new)
		{
			i->OnLayerChanged();
		}
	}

	void GameObject::OnPostRender()
	{
		for (const auto& i : m_components)
		{
			i->OnPostRender();
		}

		for (const auto& i : m_components_new)
		{
			i->OnPostRender();
		}
	}
}
