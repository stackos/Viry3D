#include "Physics.h"
#include "Debug.h"
#include "GTTime.h"
#include "GameObject.h"
#include "LayerMask.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include <algorithm>

namespace Galaxy3D
{
    static btDefaultCollisionConfiguration *g_config = NULL;
    static btCollisionDispatcher *g_dispatcher = NULL;
    static btDbvtBroadphase *g_broadphase = NULL;
    static btSequentialImpulseConstraintSolver *g_solver = NULL;
    static btDiscreteDynamicsWorld *g_dynamics_world = NULL;
    static Vector3 g_hit_from;

    static bool hit_less(const RaycastHit &a, const RaycastHit &b)
    {
        return (a.point - g_hit_from).SqrMagnitude() < (b.point - g_hit_from).SqrMagnitude();
    }

    void Physics::Init()
    {
        if(g_config != NULL)
        {
            Debug::Log("Physics had init error");
        }
        
        g_config = new btDefaultCollisionConfiguration();
        g_dispatcher = new btCollisionDispatcher(g_config);
        g_broadphase = new btDbvtBroadphase();
        g_solver = new btSequentialImpulseConstraintSolver();
        g_dynamics_world = new btDiscreteDynamicsWorld(g_dispatcher, g_broadphase, g_solver, g_config);
        g_dynamics_world->setGravity(btVector3(0, -10, 0));
        g_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    }

    void Physics::AddRigidBody(void *body)
    {
        g_dynamics_world->addRigidBody((btRigidBody *) body);
    }

    void Physics::RemoveRigidBody(void *body)
    {
        g_dynamics_world->removeRigidBody((btRigidBody *) body);
    }

    void Physics::AddCharacter(void *character)
    {
        auto c = (btKinematicCharacterController *) character;
        g_dynamics_world->addCharacter(c);

        auto ghost = c->getGhostObject();
        g_dynamics_world->addCollisionObject(
            ghost,
            btBroadphaseProxy::CharacterFilter,
            btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
    }

    void Physics::RemoveCharacter(void *character)
    {
        auto c = (btKinematicCharacterController *) character;
        g_dynamics_world->removeCharacter(c);

        auto ghost = c->getGhostObject();
        g_dynamics_world->removeCollisionObject(ghost);
    }

    bool Physics::Raycast(const Vector3 &from, const Vector3 &dir, float length, RaycastHit &hit)
    {
        Vector3 to = from + Vector3::Normalize(dir) * length;
        btVector3 from_(from.x, from.y, from.z);
        btVector3 to_(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback closest(from_, to_);
        closest.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

        g_dynamics_world->rayTest(from_, to_, closest);

        if(closest.hasHit())
        {
            btVector3 pos = from_.lerp(to_, closest.m_closestHitFraction);
            btVector3 nor = closest.m_hitNormalWorld;

            hit.point = Vector3(pos.x(), pos.y(), pos.z());
            hit.normal = Vector3(nor.x(), nor.y(), nor.z());

            if(closest.m_collisionObject != NULL)
            {
                void *user_data = closest.m_collisionObject->getUserPointer();
                if(user_data != NULL)
                {
                    Collider *collider = (Collider *) user_data;
                    hit.collider = std::dynamic_pointer_cast<Collider>(collider->GetComponentPtr());
                }
            }

            return true;
        }

        return false;
    }

    std::vector<RaycastHit> Physics::RaycastAll(const Vector3 &from, const Vector3 &dir, float length, int layer_mask)
    {
        std::vector<RaycastHit> hits;

        Vector3 to = from + Vector3::Normalize(dir) * length;
        btVector3 from_(from.x, from.y, from.z);
        btVector3 to_(to.x, to.y, to.z);

        btCollisionWorld::AllHitsRayResultCallback all(from_, to_);
        all.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

        g_dynamics_world->rayTest(from_, to_, all);

        if(all.hasHit())
        {
            for(int i=0; i<all.m_hitFractions.size(); i++)
            {
                RaycastHit hit;

                btVector3 pos = from_.lerp(to_, all.m_hitFractions[i]);
                btVector3 nor = all.m_hitNormalWorld[i];

                hit.point = Vector3(pos.x(), pos.y(), pos.z());
                hit.normal = Vector3(nor.x(), nor.y(), nor.z());

                if(all.m_collisionObjects[i] != NULL)
                {
                    void *user_data = all.m_collisionObjects[i]->getUserPointer();
                    if(user_data != NULL)
                    {
                        Collider *collider = (Collider *) user_data;
                        hit.collider = std::dynamic_pointer_cast<Collider>(collider->GetComponentPtr());

                        int obj_layer = hit.collider.lock()->GetGameObject()->GetLayer();
                        if(!LayerMask::IsCulled(obj_layer, layer_mask))
                        {
                            hits.push_back(hit);
                        }
                    }
                }
            }
        }

        // sort result by distance
        if(hits.size() > 1)
        {
            g_hit_from = from;
            std::sort(hits.begin(), hits.end(), hit_less);
        }

        return hits;
    }

    void Physics::Step()
    {
        g_dynamics_world->stepSimulation(GTTime::GetDeltaTime());
    }

    void Physics::Done()
    {
        delete g_dynamics_world;
        delete g_solver;
        delete g_broadphase;
        delete g_dispatcher;
        delete g_config;
    }
}