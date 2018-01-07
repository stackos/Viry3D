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

#include "MeshCollider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(MeshCollider);

	void MeshCollider::DeepCopy(const Ref<Object>& source)
	{
		Collider::DeepCopy(source);

		auto src = RefCast<MeshCollider>(source);
		this->m_mesh = src->m_mesh;
	}

	MeshCollider::~MeshCollider()
	{
		if (m_collider_data != NULL)
		{
			delete (btTriangleIndexVertexArray*) m_collider_data;
			m_collider_data = NULL;
		}

		if (m_indices != NULL)
		{
			delete[] m_indices;
			m_indices = NULL;
		}

		if (m_vertices != NULL)
		{
			delete[] m_vertices;
			m_vertices = NULL;
		}
	}

	void MeshCollider::SetIsRigidbody(bool value)
	{
		m_is_rigidbody = false;
	}

	void MeshCollider::Start()
	{
		int index_count = 0;
		int submesh = m_mesh->GetSubmeshCount();
		for (int i = 0; i < submesh; i++)
		{
			int start, count;
			m_mesh->GetIndexRange(i, start, count);
			index_count += count;
		}

		int idnex_size = index_count * sizeof(unsigned short);
		m_indices = new unsigned short[idnex_size];

		int old_size = 0;
		for (int i = 0; i < submesh; i++)
		{
			int start, count;
			m_mesh->GetIndexRange(i, start, count);
			memcpy(&m_indices[old_size], &m_mesh->triangles[start], count * sizeof(unsigned short));
			old_size += count;
		}

		const auto& vertices = m_mesh->vertices;
		m_vertices = new Vector3[vertices.Size()];
        memcpy(m_vertices, vertices.Bytes(), vertices.SizeInBytes());

		btIndexedMesh mesh;
		mesh.m_numTriangles = index_count / 3;
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

	void MeshCollider::OnTranformChanged()
	{
		if (m_collider != NULL)
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
