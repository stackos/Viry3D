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

#include "BoxCollider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Viry3D
{
    BoxCollider::BoxCollider():
        m_center(0, 0, 0),
        m_size(1, 1, 1)
	{
		
	}

    void BoxCollider::Start()
    {
        auto pos = this->GetTransform()->GetPosition();
        auto rot = this->GetTransform()->GetRotation();
        auto sca = this->GetTransform()->GetScale();
        sca = Vector3::Max(sca, Vector3::One() * 0.001f);

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

        auto shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
        shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));

        if (m_is_rigidbody)
        {
            btScalar mass(1);
            btVector3 local_inertia(0, 0, 0);
            shape->calculateLocalInertia(mass, local_inertia);
            auto motion_state = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo info(mass, motion_state, shape, local_inertia);

            auto body = new btRigidBody(info);
            body->setRollingFriction(0.1f);
            body->setFriction(0.5f);
            body->setUserPointer(this);

            Physics::AddRigidBody(body);

            auto proxy = body->getBroadphaseHandle();
            proxy->layer = this->GetGameObject()->GetLayer();

            m_collider = body;
        }
        else
        {
            auto col = new btCollisionObject();
            col->setWorldTransform(transform);
            col->setCollisionShape(shape);
            col->setRollingFriction(0.1f);
            col->setFriction(0.5f);
            col->setUserPointer(this);

            Physics::AddCollider(col);

            auto proxy = col->getBroadphaseHandle();
            proxy->layer = this->GetGameObject()->GetLayer();

            m_collider = col;
        }

        m_in_world = true;
    }

	void BoxCollider::Update()
	{
		if (m_is_rigidbody)
		{
			auto body = (btRigidBody*) m_collider;
			auto state = body->getMotionState();
			if (state)
			{
				auto sca = this->GetTransform()->GetScale();
                sca = Vector3::Max(sca, Vector3::One() * 0.001f);
                
				btTransform transform;
				state->getWorldTransform(transform);

				auto origin = transform.getOrigin();
				auto rot = transform.getRotation();

				Vector3 pos;
				pos.x = origin.x() - m_center.x * sca.x;
				pos.y = origin.y() - m_center.y * sca.y;
				pos.z = origin.z() - m_center.z * sca.z;

				this->GetTransform()->SetPosition(pos);
				this->GetTransform()->SetRotation(Quaternion(rot.x(), rot.y(), rot.z(), rot.w()));
			}
		}
	}

	void BoxCollider::SetCenter(const Vector3& center)
	{
		m_center = center;

		if (m_collider != nullptr)
		{
			auto pos = this->GetTransform()->GetPosition();
			auto rot = this->GetTransform()->GetRotation();
			auto sca = this->GetTransform()->GetScale();
            sca = Vector3::Max(sca, Vector3::One() * 0.001f);
            
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
			transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

			auto col = (btCollisionObject*) m_collider;
			col->setWorldTransform(transform);
		}
	}

	void BoxCollider::SetSize(const Vector3& size)
	{
		m_size = size;

		if (m_collider != nullptr)
		{
			auto sca = this->GetTransform()->GetScale();
            sca = Vector3::Max(sca, Vector3::One() * 0.001f);

			auto col = (btCollisionObject*) m_collider;
			auto shape = col->getCollisionShape();
			shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));
		}
	}

	void BoxCollider::OnTransformDirty()
	{
		if (m_collider != nullptr)
		{
			if (m_is_rigidbody == false)
			{
				auto pos = this->GetTransform()->GetPosition();
				auto rot = this->GetTransform()->GetRotation();
				auto sca = this->GetTransform()->GetScale();
                sca = Vector3::Max(sca, Vector3::One() * 0.001f);
                
				auto col = (btCollisionObject*) m_collider;
				auto shape = col->getCollisionShape();
				shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));

				btTransform transform;
				transform.setIdentity();
				transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
				transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

				col->setWorldTransform(transform);
			}
		}
	}
}
