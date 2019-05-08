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
		return obj;
	}
	
	void GameObject::Destroy(Ref<GameObject>& obj)
	{
		Scene::Instance()->RemoveGameObject(obj);
		obj.reset();
	}

	GameObject::GameObject(const String& name):
		m_is_active_self(true),
		m_is_active_in_tree(true)
	{
		this->SetName(name);
	}
    
	GameObject::~GameObject()
    {
        
    }

	void GameObject::Update()
	{
	
	}
}
