#include "CharacterController.h"
#include "Transform.h"
#include "Physics.h"
#include "LayerMask.h"
#include "Layer.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

namespace Galaxy3D
{
    CharacterController::~CharacterController()
    {
        if(m_character != NULL)
        {
            OnDisable();

            auto c = (btKinematicCharacterController *) m_character;
            
            auto g = c->getGhostObject();
            if(g != NULL)
            {
                auto s = g->getCollisionShape();
                if(s != NULL)
                {
                    delete s;
                }

                delete g;
            }

            delete c;
        }
    }

    void CharacterController::Start()
    {
        auto pos = GetTransform()->GetPosition();

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(pos.x + m_center.x, pos.y + m_center.y, pos.z + m_center.z));

        btPairCachingGhostObject* ghost_object = new btPairCachingGhostObject();
        ghost_object->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        ghost_object->setWorldTransform(transform);

        btConvexShape *shape = new btCapsuleShape(m_radius, m_height - m_radius * 2); 
        ghost_object->setCollisionShape(shape);
        
        btKinematicCharacterController *character = new btKinematicCharacterController(ghost_object, shape, m_step_height);
        character->setGravity(9.8f * 10);

        Physics::AddCharacter(character);

        m_in_world = true;
        m_character = character;
    }

    void CharacterController::OnTranformChanged()
    {
        if(m_character != NULL)
        {
            auto pos = GetTransform()->GetPosition();
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(pos.x + m_center.x, pos.y + m_center.y, pos.z + m_center.z));

            auto c = (btKinematicCharacterController *) m_character;
            c->getGhostObject()->setWorldTransform(transform);
        }
    }

    void CharacterController::OnEnable()
    {
        if(!m_in_world)
        {
            m_in_world = true;

            auto c = (btKinematicCharacterController *) m_character;
            Physics::AddCharacter(c);
        }
    }

    void CharacterController::OnDisable()
    {
        if(m_in_world)
        {
            m_in_world = false;

            auto c = (btKinematicCharacterController *) m_character;
            Physics::RemoveCharacter(c);
        }
    }

    void CharacterController::Move(const Vector3 &offset)
    {
        auto c = (btKinematicCharacterController *) m_character;
        c->setWalkDirection(btVector3(offset.x, offset.y, offset.z));
    }

    void CharacterController::Update()
    {
        auto c = (btKinematicCharacterController *) m_character;
        auto pos = c->getGhostObject()->getWorldTransform().getOrigin();
        auto target_pos = Vector3(pos.x() - m_center.x, pos.y() - m_center.y, pos.z() - m_center.z);
        
        // keep up on ground
        Vector3 from = target_pos + Vector3(0, 100, 0);
        auto hits = Physics::RaycastAll(from, Vector3(0, -1, 0), 200, LayerMask::GetMask(Layer::Terrain));
        if(!hits.empty())
        {
            if(target_pos.y < hits[0].point.y)
            {
                target_pos.y = hits[0].point.y;
            }
        }

        GetTransform()->SetPosition(target_pos);
    }
}