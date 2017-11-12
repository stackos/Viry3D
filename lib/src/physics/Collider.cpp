#include "Collider.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"

namespace Galaxy3D
{
    Collider::~Collider()
    {
        btRigidBody *body = (btRigidBody *) m_rigidbody;
        if(body != NULL)
        {
            OnDisable();

            auto motion_state = body->getMotionState();
            if(motion_state != NULL)
            {
                delete motion_state;
            }

            auto shape = body->getCollisionShape();
            if(shape != NULL)
            {
                delete shape;
            }

            delete body;
        }
    }

    void Collider::OnEnable()
    {
        if(!m_in_world)
        {
            m_in_world = true;

            btRigidBody *body = (btRigidBody *) m_rigidbody;
			if(body != NULL)
			{
				Physics::AddRigidBody(body);
			}
        }
    }

    void Collider::OnDisable()
    {
        if(m_in_world)
        {
            m_in_world = false;

            btRigidBody *body = (btRigidBody *) m_rigidbody;
            Physics::RemoveRigidBody(body);
        }
    }
}