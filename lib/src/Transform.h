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

#include "Component.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
	class Transform: public Component
	{
		DECLARE_COM_CLASS(Transform, Component);

	private:
		friend class GameObject;

	public:
		WeakRef<Transform> GetParent() const { return m_parent; }
		void SetParent(const WeakRef<Transform>& parent);
		String PathInParent(const Ref<Transform>& parent) const;
		bool IsRoot() const { return m_parent.expired(); }
		int GetChildCount() const { return m_children.Size(); }
		Ref<Transform> GetChild(int index) const;
		Ref<Transform> Find(const String& path) const;
		void SetLocalPosition(const Vector3& pos);
		const Vector3& GetLocalPosition() const { return m_local_position; }
		void SetLocalRotation(const Quaternion& rot);
		const Quaternion& GetLocalRotation() const { return m_local_rotation; }
		void SetLocalScale(const Vector3& sca);
		const Vector3& GetLocalScale() const { return m_local_scale; }
		void SetPosition(const Vector3& pos);
		const Vector3& GetPosition();
		void SetRotation(const Quaternion& rot);
		const Quaternion& GetRotation();
		void SetScale(const Vector3& sca);
		const Vector3& GetScale();
		void SetLocalPositionDirect(const Vector3& pos) { m_local_position = pos; }
		void SetLocalRotationDirect(const Quaternion& rot) { m_local_rotation = rot; }
		void SetLocalScaleDirect(const Vector3& sca) { m_local_scale = sca; }
		Vector3 TransformPoint(const Vector3& point);
		Vector3 TransformDirection(const Vector3& dir);
		Vector3 InverseTransformPoint(const Vector3& point);
		Vector3 InverseTransformDirection(const Vector3& dir);
		const Matrix4x4& GetLocalToWorldMatrix();
		const Matrix4x4& GetWorldToLocalMatrix();
        void SetLocalToWorldMatrixExternal(const Matrix4x4& mat);
		Vector3 GetRight();
		Vector3 GetUp();
		Vector3 GetForward();
		void SetForward(const Vector3& forward);
		void Changed();
		bool IsChangeNotifying() const { return m_change_notifying; }

	private:
		Transform();
		void RemoveChild(const Ref<Transform>& child);
		void AddChild(const Ref<Transform>& child);
		void ApplyChange();
		void NotifyChange();
		void NotifyParentHierarchyChange();
		void NotifyChildHierarchyChange();

		WeakRef<Transform> m_parent;
		Vector<Ref<GameObject>> m_children;
		Vector3 m_local_position;
		Quaternion m_local_rotation;
		Vector3 m_local_scale;
		bool m_changed;
		Vector3 m_position;
		Quaternion m_rotation;
		Vector3 m_scale;
		Matrix4x4 m_local_to_world_matrix;
		Matrix4x4 m_world_to_local_matrix;
		bool m_change_notifying;
	};
}
