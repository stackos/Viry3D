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

#include "Transform.h"
#include "GameObject.h"

namespace Viry3D
{
	Transform::Transform():
		m_local_position(0, 0, 0),
		m_local_rotation(Quaternion::Identity()),
		m_local_scale(1, 1, 1),
		m_position(m_local_position),
		m_rotation(m_local_rotation),
		m_scale(m_local_scale),
		m_local_to_world(Matrix4x4::Identity()),
		m_world_to_local(Matrix4x4::Identity()),
		m_dirty(false),
		m_notify_children_on_dirty(true)
	{

	}
    
    Transform::~Transform()
    {
        
    }

	void Transform::SetParent(const Ref<Transform>& parent)
	{
		
	}

	Ref<Transform> Transform::Find(const String& path) const
	{
		if (path.Empty())
		{
			return Ref<Transform>();
		}

		Ref<Transform> find;
		const Transform* p = this;

		auto layers = path.Split("/");
		for (int i = 0; i < layers.Size(); ++i)
		{
			bool find_child = false;

			for (int j = 0; j < p->GetChildCount(); ++j)
			{
				if (p->GetChild(j)->GetName() == layers[i])
				{
					find_child = true;
					find = p->GetChild(j);
					p = find.get();
					break;
				}
			}

			if (!find_child)
			{
				return Ref<Transform>();
			}
		}

		return find;
	}

	void Transform::SetLocalPosition(const Vector3& pos)
	{
		
	}

	void Transform::SetLocalRotation(const Quaternion& rot)
	{
		
	}

	void Transform::SetLocalScale(const Vector3& scale)
	{
		
	}

	/*
	const Vector3& Transform::GetPosition()
	{
	
	}

	const Quaternion& Transform::GetRotation()
	{
	
	}

	const Vector3& Transform::GetScale()
	{
	
	}

	const Matrix4x4& Transform::GetLocalToWorldMatrix()
	{
	
	}

	const Matrix4x4& Transform::GetWorldToLocalMatrix()
	{
	
	}

	Vector3 Transform::GetRight()
	{
	
	}

	Vector3 Transform::GetUp()
	{
	
	}

	Vector3 Transform::GetForward()
	{
		
	}
	*/

	void Transform::MarkDirty()
	{
		m_dirty = true;
		this->GetGameObject()->OnTransformDirty();

		if (m_notify_children_on_dirty)
		{
			for (auto& i : m_children)
			{
				i->MarkDirty();
			}
		}
	}
}
