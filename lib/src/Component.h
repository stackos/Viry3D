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
#include "ComponentClassMap.h"

namespace Viry3D
{
	class Transform;
	class GameObject;

	class Component: public Object
	{
		DECLARE_COM_BASE(Component);

	private:
		friend class GameObject;

	public:
		//
		//	在这个函数里面注册所有的组件类
		//
		static void RegisterComponents();
		static void Destroy(const Ref<Component>& com);

		virtual ~Component() { }
		Ref<GameObject> GetGameObject() const;
		Ref<Transform> GetTransform() const;
		Ref<Component> GetRef() const;
		void Enable(bool enable);
		bool IsEnable() const { return m_enable; }
		bool IsStarted() const { return m_started; }
		bool IsComponent(const String& type) const;

		void SetName(const String& name);

	protected:
		Component();
		//	没有OnDestroy，用析构函数
		virtual void Awake() { }
		virtual void Start() { }
		virtual void Update() { }
		virtual void LateUpdate() { }
		virtual void OnEnable() { }
		virtual void OnDisable() { }
		virtual void OnTranformChanged() { }
		virtual void OnTranformHierarchyChanged() { }
		virtual void OnLayerChanged() { }
		virtual void OnPostRender() { }

		WeakRef<GameObject> m_gameobject;
		WeakRef<Transform> m_transform;

	private:
		void Delete();

		bool m_deleted;
		bool m_started;
		bool m_enable;
	};
}
