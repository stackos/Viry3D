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
		m_dirty(false)
	{

	}
    
    Transform::~Transform()
    {
        
    }

	void Transform::SetParent(const Ref<Transform>& parent)
	{
        Vector3 position = this->GetPosition();
        Quaternion rotation = this->GetRotation();
        Vector3 scale = this->GetScale();
        
        auto old_parent = m_parent.lock();
		if (old_parent)
        {
            for (int i = 0; i < old_parent->GetChildCount(); ++i)
            {
                if (old_parent->GetChild(i).get() == this)
                {
                    old_parent->m_children.Remove(i);
                    break;
                }
            }
			m_parent.reset();
        }
        
        if (parent)
        {
            parent->m_children.Add(this->GetGameObject()->GetTransform());
			m_parent = parent;
        }
        
        this->SetPosition(position);
        this->SetRotation(rotation);
        this->SetScale(scale);
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

	Ref<Transform> Transform::GetRoot() const
	{
		auto parent = this->GetParent();
		if (parent)
		{
			return parent->GetRoot();
		}
		else
		{
			return this->GetGameObject()->GetTransform();
		}
	}

	void Transform::SetLocalPosition(const Vector3& pos)
	{
        m_local_position = pos;
        
        this->MarkDirty();
	}

	void Transform::SetLocalRotation(const Quaternion& rot)
	{
        m_local_rotation = rot;
        
        this->MarkDirty();
	}

	void Transform::SetLocalScale(const Vector3& scale)
	{
        m_local_scale = scale;
        
        this->MarkDirty();
	}

	const Vector3& Transform::GetPosition()
	{
        this->UpdateMatrix();
        
        return m_position;
	}
    
    void Transform::SetPosition(const Vector3& pos)
    {
		Vector3 local_position;

		auto parent = m_parent.lock();
		if (parent)
		{
			local_position = parent->GetWorldToLocalMatrix().MultiplyPoint3x4(pos);
		}
		else
		{
			local_position = pos;
		}

		this->SetLocalPosition(local_position);
    }

	const Quaternion& Transform::GetRotation()
	{
        this->UpdateMatrix();
        
        return m_rotation;
	}
    
    void Transform::SetRotation(const Quaternion& rot)
    {
        Quaternion local_rotation;
        
        auto parent = m_parent.lock();
        if (parent)
        {
            local_rotation = Quaternion::Inverse(parent->GetRotation()) * rot;
        }
        else
        {
            local_rotation = rot;
        }
        
        this->SetLocalRotation(local_rotation);
    }

	const Vector3& Transform::GetScale()
	{
        this->UpdateMatrix();
        
        return m_scale;
	}

    void Transform::SetScale(const Vector3& scale)
    {
        Vector3 local_scale;
        
        auto parent = m_parent.lock();
        if (parent)
        {
            const auto& parent_scale = parent->GetScale();
            local_scale = Vector3(scale.x / parent_scale.x, scale.y / parent_scale.y, scale.z / parent_scale.z);
        }
        else
        {
            local_scale = scale;
        }
        
        this->SetLocalScale(local_scale);
    }
    
	const Matrix4x4& Transform::GetLocalToWorldMatrix()
	{
		this->UpdateMatrix();
        
        return m_local_to_world;
	}

	const Matrix4x4& Transform::GetWorldToLocalMatrix()
	{
        this->UpdateMatrix();
        
        return m_world_to_local;
	}

	Vector3 Transform::GetRight()
	{
        this->UpdateMatrix();
        
        return m_local_to_world.MultiplyDirection(Vector3(1, 0, 0));
	}

	Vector3 Transform::GetUp()
	{
        this->UpdateMatrix();
        
        return m_local_to_world.MultiplyDirection(Vector3(0, 1, 0));
	}

	Vector3 Transform::GetForward()
	{
        this->UpdateMatrix();
        
        return m_local_to_world.MultiplyDirection(Vector3(0, 0, 1));
	}

	void Transform::MarkDirty()
	{
		m_dirty = true;
		this->GetGameObject()->OnTransformDirty();

		for (auto& i : m_children)
		{
			i->MarkDirty();
		}
	}

	void Transform::UpdateMatrix()
	{
		if (m_dirty)
		{
			m_dirty = false;

			auto parent = m_parent.lock();
			if (parent)
			{
				m_local_to_world = parent->GetLocalToWorldMatrix() * Matrix4x4::TRS(m_local_position, m_local_rotation, m_local_scale);
				m_position = parent->GetLocalToWorldMatrix().MultiplyPoint3x4(m_local_position);
				m_rotation = parent->GetRotation() * m_local_rotation;
				const auto& parent_scale = parent->GetScale();
				m_scale = Vector3(parent_scale.x * m_local_scale.x, parent_scale.y * m_local_scale.y, parent_scale.z * m_local_scale.z);
			}
			else
			{
				m_local_to_world = Matrix4x4::TRS(m_local_position, m_local_rotation, m_local_scale);
				m_position = m_local_position;
				m_rotation = m_local_rotation;
				m_scale = m_local_scale;
			}

			m_world_to_local = m_local_to_world.Inverse();
		}
	}
}
