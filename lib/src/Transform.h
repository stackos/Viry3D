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

#include "Component.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
    class Transform : public Component
    {
    public:
        Transform();
        virtual ~Transform();
		Ref<Transform> GetParent() const { return m_parent.lock(); }
		void SetParent(const Ref<Transform>& parent);
		int GetChildCount() const { return m_children.Size(); }
		const Ref<Transform>& GetChild(int index) const { return m_children[index]; }
		Ref<Transform> Find(const String& path) const;
		Ref<Transform> GetRoot() const;
		const Vector3& GetLocalPosition() const { return m_local_position; }
		void SetLocalPosition(const Vector3& pos);
		const Quaternion& GetLocalRotation() const { return m_local_rotation; }
		void SetLocalRotation(const Quaternion& rot);
		const Vector3& GetLocalScale() const { return m_local_scale; }
		void SetLocalScale(const Vector3& scale);
		const Vector3& GetPosition();
        void SetPosition(const Vector3& pos);
		const Quaternion& GetRotation();
        void SetRotation(const Quaternion& rot);
		const Vector3& GetScale();
        void SetScale(const Vector3& scale);
		const Matrix4x4& GetLocalToWorldMatrix();
		const Matrix4x4& GetWorldToLocalMatrix();
		Vector3 GetRight();
		Vector3 GetUp();
		Vector3 GetForward();

	private:
		void MarkDirty();

	private:
		WeakRef<Transform> m_parent;
		Vector<Ref<Transform>> m_children;
		Vector3 m_local_position;
		Quaternion m_local_rotation;
		Vector3 m_local_scale;
		Vector3 m_position;
		Quaternion m_rotation;
		Vector3 m_scale;
		Matrix4x4 m_local_to_world;
		Matrix4x4 m_world_to_local;
		bool m_dirty;
    };
}
