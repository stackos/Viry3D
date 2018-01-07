/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
	DEFINE_COM_CLASS(Transform);

	Transform::Transform():
		m_local_position(0, 0, 0),
		m_local_rotation(0, 0, 0, 1),
		m_local_scale(1, 1, 1),
		m_changed(true),
		m_position(m_local_position),
		m_rotation(m_local_rotation),
		m_scale(m_local_scale),
		m_change_notifying(false)
	{
	}

	void Transform::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<Transform>(source);
		m_changed = true;
		m_position = src->GetPosition();
		m_rotation = src->GetRotation();
		m_scale = src->GetScale();
		SetLocalPosition(m_position);
		SetLocalRotation(m_rotation);
		SetLocalScale(m_scale);

		for (int i = 0; i < src->m_children.Size(); i++)
		{
			const auto& src_child = src->m_children[i];
			auto child = GameObject::Instantiate(src_child);

			child->GetTransform()->SetParent(RefCast<Transform>(GetRef()));
		}
	}

	void Transform::RemoveChild(const Ref<Transform>& child)
	{
		for (int i = 0; i < m_children.Size(); i++)
		{
			if (m_children[i]->GetTransform() == child)
			{
				m_children.Remove(i);
				break;
			}
		}
	}

	void Transform::AddChild(const Ref<Transform>& child)
	{
		m_children.Add(child->GetGameObject());
	}

	String Transform::PathInParent(const Ref<Transform>& parent) const
	{
		String path = this->GetName();

		auto t = this->GetParent();
		while (!t.expired() && t.lock() != parent)
		{
			path = t.lock()->GetName() + "/" + path;
			t = t.lock()->GetParent();
		}

		if (t.expired())
		{
			path = "";
		}

		return path;
	}

	void Transform::SetParent(const WeakRef<Transform>& parent)
	{
		auto obj = GetGameObject();

		ApplyChange();

		if (!m_parent.expired())
		{
			auto p = m_parent.lock();
			p->RemoveChild(m_transform.lock());
			p->NotifyParentHierarchyChange();
			m_parent.reset();

			//	become root
			if (parent.expired())
			{
				m_local_position = m_position;
				m_local_rotation = m_rotation;
				m_local_scale = m_scale;
				this->Changed();
				this->NotifyChildHierarchyChange();

				obj->SetActiveInHierarchy(obj->IsActiveSelf());
			}
		}

		m_parent = parent;

		if (!m_parent.expired())
		{
			auto p = m_parent.lock();
			p->AddChild(m_transform.lock());
			p->NotifyParentHierarchyChange();

			//become child
			{
				m_local_position = p->InverseTransformPoint(m_position);
				m_local_rotation = Quaternion::Inverse(p->GetRotation()) * m_rotation;
				const Vector3& parent_scale = p->GetScale();
				float x = m_scale.x / parent_scale.x;
				float y = m_scale.y / parent_scale.y;
				float z = m_scale.z / parent_scale.z;
				m_local_scale = Vector3(x, y, z);
				this->Changed();
				this->NotifyChildHierarchyChange();

				obj->SetActiveInHierarchy(p->GetGameObject()->IsActiveInHierarchy() && obj->IsActiveSelf());
			}
		}
	}

	Ref<Transform> Transform::GetChild(int index) const
	{
		return m_children[index]->GetTransform();
	}

	Ref<Transform> Transform::Find(const String& path) const
	{
		Ref<Transform> find;

		String p = path;
		auto names = p.Split("/");

		for (auto& c : m_children)
		{
            auto child = c->GetTransform();
			auto name = names[0];

			if (child->GetName() == name)
			{
				if (names.Size() > 1)
				{
					find = child->Find(path.Substring(name.Size() + 1));
				}
				else
				{
					find = child;
				}

				break;
			}
		}

		return find;
	}

	void Transform::NotifyChange()
	{
		m_change_notifying = true;

		GetGameObject()->OnTranformChanged();

		for (auto& i : m_children)
		{
			i->GetTransform()->NotifyChange();
		}

		m_change_notifying = false;
	}

	void Transform::NotifyParentHierarchyChange()
	{
		m_change_notifying = true;

		GetGameObject()->OnTranformHierarchyChanged();

		auto p = m_parent;
		while (!p.expired())
		{
			auto parent = p.lock();
			parent->NotifyParentHierarchyChange();
			p = parent->GetParent();
		}

		m_change_notifying = false;
	}

	void Transform::NotifyChildHierarchyChange()
	{
		m_change_notifying = true;

		GetGameObject()->OnTranformHierarchyChanged();

		for (auto& i : m_children)
		{
			i->GetTransform()->NotifyChildHierarchyChange();
		}

		m_change_notifying = false;
	}

	void Transform::SetLocalPosition(const Vector3& pos)
	{
		if (m_local_position != pos)
		{
			m_local_position = pos;
			Changed();
			NotifyChange();
		}
	}

	void Transform::SetLocalRotation(const Quaternion& rot)
	{
		Quaternion r = rot;
		r.Normalize();

		if (m_local_rotation != r)
		{
			m_local_rotation = r;
			Changed();
			NotifyChange();
		}
	}

	void Transform::SetLocalScale(const Vector3& sca)
	{
		if (m_local_scale != sca)
		{
			m_local_scale = sca;
			Changed();
			NotifyChange();
		}
	}

	void Transform::SetPosition(const Vector3& pos)
	{
		if (!m_changed && m_position == pos)
		{
			return;
		}

		if (IsRoot())
		{
			SetLocalPosition(pos);
		}
		else
		{
			Vector3 local = m_parent.lock()->InverseTransformPoint(pos);
			SetLocalPosition(local);
		}
	}

	const Vector3& Transform::GetPosition()
	{
		ApplyChange();

		return m_position;
	}

	void Transform::SetRotation(const Quaternion& rot)
	{
		if (!m_changed && m_rotation == rot)
		{
			return;
		}

		if (IsRoot())
		{
			SetLocalRotation(rot);
		}
		else
		{
			Quaternion local = Quaternion::Inverse(m_parent.lock()->GetRotation()) * rot;
			SetLocalRotation(local);
		}
	}

	const Quaternion& Transform::GetRotation()
	{
		ApplyChange();

		return m_rotation;
	}

	void Transform::SetScale(const Vector3& sca)
	{
		if (!m_changed && m_scale == sca)
		{
			return;
		}

		if (IsRoot())
		{
			SetLocalScale(sca);
		}
		else
		{
			const Vector3& parent_scale = m_parent.lock()->GetScale();
			float x = sca.x / parent_scale.x;
			float y = sca.y / parent_scale.y;
			float z = sca.z / parent_scale.z;
			SetLocalScale(Vector3(x, y, z));
		}
	}

	const Vector3& Transform::GetScale()
	{
		ApplyChange();

		return m_scale;
	}

	void Transform::Changed()
	{
		m_changed = true;
		for (auto& i : m_children)
		{
			i->GetTransform()->Changed();
		}
	}

	void Transform::ApplyChange()
	{
		if (m_changed)
		{
			m_changed = false;

			if (IsRoot())
			{
				m_position = m_local_position;
				m_rotation = m_local_rotation;
				m_scale = m_local_scale;
			}
			else
			{
				auto parent = m_parent.lock();

				m_position = parent->TransformPoint(m_local_position);
				m_rotation = parent->GetRotation() * m_local_rotation;

				const Vector3& ps = parent->GetScale();
				float x = m_local_scale.x * ps.x;
				float y = m_local_scale.y * ps.y;
				float z = m_local_scale.z * ps.z;
				m_scale = Vector3(x, y, z);
			}

			m_local_to_world_matrix = Matrix4x4::TRS(m_position, m_rotation, m_scale);
		}
	}

	Vector3 Transform::TransformPoint(const Vector3& point)
	{
		return GetLocalToWorldMatrix().MultiplyPoint3x4(point);
	}

	Vector3 Transform::TransformDirection(const Vector3& dir)
	{
		return GetLocalToWorldMatrix().MultiplyDirection(dir);
	}

	Vector3 Transform::InverseTransformPoint(const Vector3& point)
	{
		return GetWorldToLocalMatrix().MultiplyPoint3x4(point);
	}

	Vector3 Transform::InverseTransformDirection(const Vector3& dir)
	{
		return GetWorldToLocalMatrix().MultiplyDirection(dir);
	}

	const Matrix4x4& Transform::GetLocalToWorldMatrix()
	{
		ApplyChange();

		return m_local_to_world_matrix;
	}

	const Matrix4x4& Transform::GetWorldToLocalMatrix()
	{
		m_world_to_local_matrix = GetLocalToWorldMatrix().Inverse();

		return m_world_to_local_matrix;
	}

    void Transform::SetLocalToWorldMatrixExternal(const Matrix4x4& mat)
    {
        this->SetPosition(mat.MultiplyPoint3x4(Vector3::Zero()));
        this->SetForward(mat.MultiplyDirection(Vector3(0, 0, 1)));
        
        float scale_x = (mat.MultiplyPoint3x4(Vector3::Zero()) - mat.MultiplyPoint3x4(Vector3(1, 0, 0))).Magnitude();
        float scale_y = (mat.MultiplyPoint3x4(Vector3::Zero()) - mat.MultiplyPoint3x4(Vector3(0, 1, 0))).Magnitude();
        float scale_z = (mat.MultiplyPoint3x4(Vector3::Zero()) - mat.MultiplyPoint3x4(Vector3(0, 0, 1))).Magnitude();
        this->SetScale(Vector3(scale_x, scale_y, scale_z));
    }
    
	Vector3 Transform::GetRight()
	{
		return GetRotation() * Vector3(1, 0, 0);
	}

	Vector3 Transform::GetUp()
	{
		return GetRotation() * Vector3(0, 1, 0);
	}

	Vector3 Transform::GetForward()
	{
		return GetRotation() * Vector3(0, 0, 1);
	}

	void Transform::SetForward(const Vector3& forward)
	{
		Vector3 origin = Vector3(0, 0, 1);
		Vector3 fn = Vector3::Normalize(forward);

		if (fn != origin)
		{
			if (!Mathf::FloatEqual(fn.SqrMagnitude(), 0))
			{
				float deg = Vector3::Angle(origin, fn);
				Vector3 axis = origin * fn;

				if (axis == Vector3(0, 0, 0))
				{
					SetRotation(Quaternion::AngleAxis(deg, GetUp()));
				}
				else
				{
					SetRotation(Quaternion::AngleAxis(deg, axis));
				}
			}
		}
		else
		{
			SetRotation(Quaternion::Identity());
		}
	}
}
