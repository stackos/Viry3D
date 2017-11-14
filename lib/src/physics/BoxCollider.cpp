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

#include "BoxCollider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(BoxCollider);

	void BoxCollider::DeepCopy(const Ref<Object>& source)
	{
		Collider::DeepCopy(source);

		auto src = RefCast<BoxCollider>(source);
		this->m_center = src->m_center;
		this->m_size = src->m_size;
	}

	void BoxCollider::Start()
	{
		auto pos = GetTransform()->GetPosition();
		auto rot = GetTransform()->GetRotation();
		auto sca = GetTransform()->GetScale();

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
		transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

		auto shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));

		auto col = new btCollisionObject();
		col->setWorldTransform(transform);
		col->setCollisionShape(shape);
		col->setRollingFriction(1);
		col->setFriction(1);
		col->setUserPointer(this);

		Physics::AddCollider(col);

		auto proxy = col->getBroadphaseHandle();
		proxy->layer = this->GetGameObject()->GetLayer();

		m_in_world = true;
		m_collider = col;
	}

	void BoxCollider::SetCenter(const Vector3 &center)
	{
		m_center = center;

		if (m_collider != NULL)
		{
			auto pos = GetTransform()->GetPosition();
			auto rot = GetTransform()->GetRotation();
			auto sca = GetTransform()->GetScale();

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

		if (m_collider != NULL)
		{
			auto sca = GetTransform()->GetScale();

			auto col = (btCollisionObject*) m_collider;
			auto shape = col->getCollisionShape();
			shape->setLocalScaling(btVector3(m_size.x * sca.x, m_size.y * sca.y, m_size.z * sca.z));
		}
	}

	void BoxCollider::OnTranformChanged()
	{
		if (m_collider != NULL)
		{
			auto pos = GetTransform()->GetPosition();
			auto rot = GetTransform()->GetRotation();
			auto sca = GetTransform()->GetScale();

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
