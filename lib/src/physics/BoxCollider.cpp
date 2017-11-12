#include "BoxCollider.h"
#include "Transform.h"
#include "GameObject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    void BoxCollider::Start()
    {
        auto pos = GetTransform()->GetPosition();
        auto rot = GetTransform()->GetRotation();
        auto sca = GetTransform()->GetScale();

        btBoxShape *shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
        Vector3 size = m_size;
        if(Mathf::FloatEqual(size.x, 0))
        {
            size.x = 1;
        }
        if(Mathf::FloatEqual(size.y, 0))
        {
            size.y = 1;
        }
        if(Mathf::FloatEqual(size.z, 0))
        {
            size.z = 1;
        }
        shape->setLocalScaling(btVector3(size.x * sca.x, size.y * sca.y, size.z * sca.z));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

        btScalar mass(0);
        btVector3 local_inertia(0, 0, 0);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState *motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, local_inertia);
        btRigidBody *body = new btRigidBody(rbInfo);
        body->setRollingFriction(1);
        body->setFriction(1);
        body->setUserPointer(this);

        Physics::AddRigidBody(body);

        m_in_world = true;
        m_rigidbody = body;
    }

    void BoxCollider::SetSize(const Vector3 &size)
    {
        m_size = size;

        if(m_rigidbody != NULL)
        {
            auto sca = GetTransform()->GetScale();

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            auto shape = body->getCollisionShape();
            Vector3 size = m_size;
            if(Mathf::FloatEqual(size.x, 0))
            {
                size.x = 1;
            }
            if(Mathf::FloatEqual(size.y, 0))
            {
                size.y = 1;
            }
            if(Mathf::FloatEqual(size.z, 0))
            {
                size.z = 1;
            }
            shape->setLocalScaling(btVector3(size.x * sca.x, size.y * sca.y, size.z * sca.z));
        }
    }

    void BoxCollider::OnTranformChanged()
    {
        if(m_rigidbody != NULL)
        {
            auto pos = GetTransform()->GetPosition();
            auto rot = GetTransform()->GetRotation();
            auto sca = GetTransform()->GetScale();

			btRigidBody *body = (btRigidBody *) m_rigidbody;
			auto shape = body->getCollisionShape();
            Vector3 size = m_size;
            if(Mathf::FloatEqual(size.x, 0))
            {
                size.x = 1;
            }
            if(Mathf::FloatEqual(size.y, 0))
            {
                size.y = 1;
            }
            if(Mathf::FloatEqual(size.z, 0))
            {
                size.z = 1;
            }
            shape->setLocalScaling(btVector3(size.x * sca.x, size.y * sca.y, size.z * sca.z));

            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(pos.x + m_center.x * sca.x, pos.y + m_center.y * sca.y, pos.z + m_center.z * sca.z));
            transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

            body->setWorldTransform(transform);
        }
    }
}