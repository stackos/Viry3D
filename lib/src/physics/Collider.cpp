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

#include "Collider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Collider);

	void Collider::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);
	}

	Collider::~Collider()
	{
		auto col = (btCollisionObject*) m_collider;
		if (col != NULL)
		{
			this->OnDisable();

			auto shape = col->getCollisionShape();
			if (shape != NULL)
			{
				delete shape;
			}

			delete col;
		}
	}

	void Collider::OnEnable()
	{
		if (!m_in_world)
		{
			m_in_world = true;

			auto col = (btCollisionObject*) m_collider;
			if (col != NULL)
			{
				Physics::AddCollider(col);

				auto proxy = col->getBroadphaseHandle();
				proxy->layer = this->GetGameObject()->GetLayer();
			}
		}
	}

	void Collider::OnDisable()
	{
		if (m_in_world)
		{
			m_in_world = false;

			auto col = (btCollisionObject*) m_collider;
			Physics::RemoveCollider(col);
		}
	}

	void Collider::OnLayerChanged()
	{
		if (m_in_world)
		{
			auto col = (btCollisionObject*) m_collider;
			auto proxy = col->getBroadphaseHandle();
			proxy->layer = this->GetGameObject()->GetLayer();
		}
	}
}
