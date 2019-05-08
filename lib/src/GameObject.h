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

namespace Viry3D
{
    class GameObject : public Object
    {
    public:
		static Ref<GameObject> Create(const String& name);
		static void Destroy(Ref<GameObject>& obj);
        virtual ~GameObject();
		void Update();
		bool IsActiveSelf() const { return m_is_active_self; }
		bool IsActiveInTree() const { return m_is_active_in_tree; }
		
	private:
		GameObject(const String& name);

	private:
		bool m_is_active_self;
		bool m_is_active_in_tree;
    };
}
