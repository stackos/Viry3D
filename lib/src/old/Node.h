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
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4x4.h"
#include "container/Vector.h"

namespace Viry3D
{
    class Node : public Object
    {
    public:
        static void SetParent(const Ref<Node>& node, const Ref<Node>& parent);
        static const Ref<Node>& GetRoot(const Ref<Node>& node);
        Node();
        virtual ~Node();
        const Vector3& GetLocalPosition() const { return m_local_position; }
        void SetLocalPosition(const Vector3& pos);
        const Quaternion& GetLocalRotation() const { return m_local_rotation; }
        void SetLocalRotation(const Quaternion& rot);
        const Vector3& GetLocalScale() const { return m_local_scale; }
        void SetLocalScale(const Vector3& scale);
        const Matrix4x4& GetLocalToWorldMatrix();
        Vector3 GetPosition();
        Quaternion GetRotation();
        Vector3 GetRight();
        Vector3 GetUp();
        Vector3 GetForward();
        Vector3 GetScale();
        Ref<Node> GetParent() const { return m_parent.lock(); }
        int GetChildCount() const { return m_children.Size(); }
        const Ref<Node>& GetChild(int index) const { return m_children[index]; }
        Ref<Node> Find(const String& path);
        void EnableNotifyChildrenOnMatrixDirty(bool enable) { m_notify_children_on_matrix_dirty = enable; }

    protected:
        virtual void OnMatrixDirty() { }

    private:
        void MarkMatrixDirty();

    private:
        Vector3 m_local_position;
        Quaternion m_local_rotation;
        Vector3 m_local_scale;
        bool m_matrix_dirty;
        bool m_notify_children_on_matrix_dirty;
        Matrix4x4 m_local_to_world_matrix;
        Vector<Ref<Node>> m_children;
        WeakRef<Node> m_parent;
    };
}
