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

#include "Node.h"

namespace Viry3D
{
    Node::Node():
        m_local_position(0, 0, 0),
        m_local_rotation(Quaternion::Identity()),
        m_local_scale(1, 1, 1),
        m_matrix_dirty(true),
        m_notify_children_on_matrix_dirty(true)
    {
        
    }
    
    Node::~Node()
    {
    
    }

    void Node::SetLocalPosition(const Vector3& pos)
    {
        m_local_position = pos;
        this->MarkMatrixDirty();
    }

    void Node::SetLocalRotation(const Quaternion& rot)
    {
        m_local_rotation = rot;
        this->MarkMatrixDirty();
    }

    void Node::SetLocalScale(const Vector3& scale)
    {
        m_local_scale = scale;
        this->MarkMatrixDirty();
    }

    void Node::MarkMatrixDirty()
    {
        m_matrix_dirty = true;
        this->OnMatrixDirty();

        if (m_notify_children_on_matrix_dirty)
        {
            for (auto& i : m_children)
            {
                i->MarkMatrixDirty();
            }
        }
    }

    const Matrix4x4& Node::GetLocalToWorldMatrix()
    {
        if (m_matrix_dirty)
        {
            m_matrix_dirty = false;

            if (!m_parent.expired())
            {
                m_local_to_world_matrix = m_parent.lock()->GetLocalToWorldMatrix() * Matrix4x4::TRS(m_local_position, m_local_rotation, m_local_scale);
            }
            else
            {
                m_local_to_world_matrix = Matrix4x4::TRS(m_local_position, m_local_rotation, m_local_scale);
            }
        }

        return m_local_to_world_matrix;
    }

    Vector3 Node::GetPosition()
    {
        return this->GetLocalToWorldMatrix().MultiplyPoint3x4(Vector3(0, 0, 0));
    }

    Quaternion Node::GetRotation()
    {
        Quaternion rotation = m_local_rotation;
        if (!m_parent.expired())
        {
            Quaternion parent_rotation = m_parent.lock()->GetRotation();
            rotation = parent_rotation * rotation;
        }
        return rotation;
    }

    Vector3 Node::GetRight()
    {
        return this->GetLocalToWorldMatrix().MultiplyDirection(Vector3(1, 0, 0));
    }

    Vector3 Node::GetUp()
    {
        return this->GetLocalToWorldMatrix().MultiplyDirection(Vector3(0, 1, 0));
    }

    Vector3 Node::GetForward()
    {
        return this->GetLocalToWorldMatrix().MultiplyDirection(Vector3(0, 0, 1));
    }

    Vector3 Node::GetScale()
    {
        Vector3 scale = m_local_scale;
        if (!m_parent.expired())
        {
            Vector3 parent_scale = m_parent.lock()->GetScale();
            scale.x *= parent_scale.x;
            scale.y *= parent_scale.y;
            scale.z *= parent_scale.z;
        }
        return scale;
    }

    void Node::SetParent(const Ref<Node>& node, const Ref<Node>& parent)
    {
        bool matrix_dirty = false;

        if (!node->m_parent.expired())
        {
            node->m_parent.lock()->m_children.Remove(node);
            node->m_parent.reset();
            matrix_dirty = true;
        }

        if (parent)
        {
            parent->m_children.Add(node);
            node->m_parent = parent;
            matrix_dirty = true;
        }

        if (matrix_dirty)
        {
            node->MarkMatrixDirty();
        }
    }

    const Ref<Node>& Node::GetRoot(const Ref<Node>& node)
    {
        if (!node->m_parent.expired())
        {
            return Node::GetRoot(node->m_parent.lock());
        }
        else
        {
            return node;
        }
    }

    Ref<Node> Node::Find(const String& path)
    {
        if (path.Empty())
        {
            return Ref<Node>();
        }

        Ref<Node> find;
        Node* p = this;

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
                return Ref<Node>();
            }
        }

        return find;
    }
}
