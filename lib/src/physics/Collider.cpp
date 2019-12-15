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

#include "Collider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Viry3D
{
	Collider::~Collider()
	{
		auto col = (btCollisionObject*) m_collider;
		if (col != nullptr)
		{
			this->OnEnable(false);

			if (m_is_rigidbody)
			{
				auto motion_state = ((btRigidBody*) col)->getMotionState();
				if (motion_state != nullptr)
				{
					delete motion_state;
				}
			}

			auto shape = col->getCollisionShape();
			if (shape != nullptr)
			{
				delete shape;
			}

			delete col;
		}
	}

	void Collider::SetIsRigidbody(bool value)
	{
		m_is_rigidbody = value;
	}

    void Collider::SetRollingFriction(float f)
    {
        if (!Mathf::FloatEqual(m_rolling_friction, f))
        {
            auto col = (btCollisionObject*) m_collider;
            if (col != nullptr)
            {
                m_rolling_friction = f;
                col->setRollingFriction(f);
            }
        }
    }

    void Collider::SetFriction(float f)
    {
        if (!Mathf::FloatEqual(m_friction, f))
        {
            auto col = (btCollisionObject*) m_collider;
            if (col != nullptr)
            {
                m_friction = f;
                col->setFriction(f);
            }
        }
    }

    void Collider::SetRestitution(float r)
    {
        if (!Mathf::FloatEqual(m_restitution, r))
        {
            auto col = (btCollisionObject*) m_collider;
            if (col != nullptr)
            {
                m_restitution = r;
                col->setRestitution(r);
            }
        }
    }

	void Collider::OnEnable(bool enable)
	{
        if (enable)
        {
            if (!m_in_world)
            {
                m_in_world = true;

                auto col = (btCollisionObject*) m_collider;
                if (col != nullptr)
                {
                    if (m_is_rigidbody)
                    {
                        Physics::AddRigidBody(col);
                    }
                    else
                    {
                        Physics::AddCollider(col);
                    }

                    auto proxy = col->getBroadphaseHandle();
                    proxy->layer = this->GetGameObject()->GetLayer();
                }
            }
        }
        else
        {
            if (m_in_world)
            {
                m_in_world = false;

                auto col = (btCollisionObject*) m_collider;
                if (m_is_rigidbody)
                {
                    Physics::RemoveRigidBody(col);
                }
                else
                {
                    Physics::RemoveCollider(col);
                }
            }
        }
	}

	void Collider::OnGameObjectLayerChanged()
	{
		if (m_in_world)
		{
			auto col = (btCollisionObject*) m_collider;
			auto proxy = col->getBroadphaseHandle();
			proxy->layer = this->GetGameObject()->GetLayer();
		}
	}
}
