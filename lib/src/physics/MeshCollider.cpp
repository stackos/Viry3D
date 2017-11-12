#include "MeshCollider.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    MeshCollider::~MeshCollider()
    {
        if(m_collider_data != NULL)
        {
            delete m_collider_data;
            m_collider_data = 0;
        }

        if(m_indices != NULL)
        {
            delete [] m_indices;
            m_indices = 0;
        }

        if(m_vertices != NULL)
        {
            delete [] m_vertices;
            m_vertices = 0;
        }
    }

    void MeshCollider::Start()
    {
        auto &vertices = m_mesh->GetVertices();
        auto &indices = m_mesh->GetIndices();

        int index_count = 0;
        for(size_t i=0; i<indices.size(); i++)
        {
            index_count += indices[i].size();
        }
        int idnex_size = index_count * sizeof(unsigned short);
        m_indices = new unsigned short[idnex_size];

        int old_size = 0;
        for(size_t i=0; i<indices.size(); i++)
        {
            int size = indices[i].size();
            memcpy(&m_indices[old_size], &indices[i][0], size * sizeof(unsigned short));
            old_size += size;
        }

        auto scale = GetTransform()->GetScale();
        m_vertices = new unsigned char[vertices.size() * sizeof(VertexMesh)];
        for(size_t i=0; i<vertices.size(); i++)
        {
            auto v = vertices[i];
            v.POSITION.x *= scale.x;
            v.POSITION.y *= scale.y;
            v.POSITION.z *= scale.z;

            memcpy(&m_vertices[i * sizeof(VertexMesh)], &v, sizeof(VertexMesh));
        }

        btIndexedMesh mesh;
        mesh.m_numTriangles = index_count / 3;
        mesh.m_triangleIndexBase = (const unsigned char *) m_indices;
        mesh.m_triangleIndexStride = sizeof(unsigned short) * 3;
        mesh.m_numVertices = vertices.size();
        mesh.m_vertexBase = m_vertices;
        mesh.m_vertexStride = sizeof(VertexMesh);

        m_collider_data = new btTriangleIndexVertexArray();
        m_collider_data->addIndexedMesh(mesh, PHY_SHORT);

        btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(m_collider_data, true);

        auto pos = GetTransform()->GetPosition();
        auto rot = GetTransform()->GetRotation();
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

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

    void MeshCollider::OnTranformChanged()
    {
        if(m_rigidbody != NULL)
        {
            auto pos = GetTransform()->GetPosition();
            auto rot = GetTransform()->GetRotation();
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            body->setWorldTransform(transform);
        }
    }
}