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

#include "MeshCollider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "graphics/Mesh.h"
#include "memory/Memory.h"

namespace Viry3D
{
    MeshCollider::MeshCollider():
        m_collider_data(nullptr),
        m_indices(nullptr),
        m_vertices(nullptr)
    {
        
    }

	MeshCollider::~MeshCollider()
	{
		if (m_collider_data != nullptr)
		{
			delete (btTriangleIndexVertexArray*) m_collider_data;
			m_collider_data = nullptr;
		}

		if (m_indices != nullptr)
		{
			delete[] m_indices;
			m_indices = nullptr;
		}

		if (m_vertices != nullptr)
		{
			delete[] m_vertices;
			m_vertices = nullptr;
		}
	}

	void MeshCollider::SetIsRigidbody(bool value)
	{
		m_is_rigidbody = false;
	}

    void MeshCollider::Start()
    {
        const auto& indices = m_mesh->GetIndices();
        m_indices = new unsigned short[indices.Size()];
        for (int i = 0; i < indices.Size(); ++i)
        {
            uint32_t index = indices[i];
            assert(index <= 0xffff);
            m_indices[i] = (uint16_t) index;
        }

        const auto& vertices = m_mesh->GetVertices();
        m_vertices = new Vector3[vertices.Size()];
        for (int i = 0; i < vertices.Size(); ++i)
        {
            m_vertices[i] = vertices[i].vertex;
        }

        btIndexedMesh mesh;
        mesh.m_numTriangles = indices.Size() / 3;
        mesh.m_triangleIndexBase = (const unsigned char*) m_indices;
        mesh.m_triangleIndexStride = sizeof(unsigned short) * 3;
        mesh.m_numVertices = vertices.Size();
        mesh.m_vertexBase = (const unsigned char*) m_vertices;
        mesh.m_vertexStride = sizeof(Vector3);

        auto collider_data = new btTriangleIndexVertexArray();
        m_collider_data = collider_data;
        collider_data->addIndexedMesh(mesh, PHY_SHORT);

        auto pos = GetTransform()->GetPosition();
        auto rot = GetTransform()->GetRotation();
        auto sca = this->GetTransform()->GetScale();

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

        auto shape = new btBvhTriangleMeshShape(collider_data, true);
        shape->setLocalScaling(btVector3(sca.x, sca.y, sca.z));

        auto col = new btCollisionObject();
        col->setWorldTransform(transform);
        col->setCollisionShape(shape);
        col->setRollingFriction(1);
        col->setFriction(1);
        col->setUserPointer(this);

        Physics::AddCollider(col);

        auto proxy = col->getBroadphaseHandle();
        proxy->layer = this->GetGameObject()->GetLayer();

        m_collider = col;
        m_in_world = true;
    }

	void MeshCollider::OnTransformDirty()
	{
		if (m_collider != nullptr)
		{
			auto pos = GetTransform()->GetPosition();
			auto rot = GetTransform()->GetRotation();
            auto sca = this->GetTransform()->GetScale();
            
            auto col = (btCollisionObject*) m_collider;
            auto shape = col->getCollisionShape();
            shape->setLocalScaling(btVector3(sca.x, sca.y, sca.z));
            
			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
			transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

			col->setWorldTransform(transform);
		}
	}
}
