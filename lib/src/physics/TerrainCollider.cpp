#include "TerrainCollider.h"
#include "Transform.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    TerrainCollider::~TerrainCollider()
    {
        if(m_collider_data != NULL)
        {
            delete m_collider_data;
            m_collider_data = 0;
        }
    }

    void TerrainCollider::Start()
    {
        auto &vertices = m_terrain->GetVertices();
        auto &indices = m_terrain->GetIndices();

        btIndexedMesh mesh;
        mesh.m_numTriangles = indices.size() / 3;
        mesh.m_triangleIndexBase = (const unsigned char *) &indices[0];
        mesh.m_triangleIndexStride = sizeof(unsigned int) * 3;
        mesh.m_numVertices = vertices.size();
        mesh.m_vertexBase = (const unsigned char *) &vertices[0];
        mesh.m_vertexStride = sizeof(VertexMesh);

        m_collider_data = new btTriangleIndexVertexArray();
        m_collider_data->addIndexedMesh(mesh, PHY_INTEGER);

        btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(m_collider_data, true);

        btTransform transform;
        transform.setIdentity();
        auto pos = GetTransform()->GetPosition();
        transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

        btScalar mass(0);
        btVector3 local_inertia(0, 0, 0);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setRollingFriction(1);
        body->setFriction(1);
        body->setUserPointer(this);

        Physics::AddRigidBody(body);
        
        m_in_world = true;
        m_rigidbody = body;
    }
}