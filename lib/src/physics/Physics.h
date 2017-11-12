#ifndef __Physics_h__
#define __Physics_h__

#include "Vector3.h"
#include "Collider.h"
#include <vector>

namespace Galaxy3D
{
    struct RaycastHit
    {
        Vector3 point;
        Vector3 normal;
        std::weak_ptr<Collider> collider;
    };

    class Physics
    {
    public:
        static void Init();
        static void Step();
        static void Done();
        static void AddRigidBody(void *body);
        static void RemoveRigidBody(void *body);
        static void AddCharacter(void *character);
        static void RemoveCharacter(void *character);
        static bool Raycast(const Vector3 &from, const Vector3 &dir, float length, RaycastHit &hit);
        static std::vector<RaycastHit> RaycastAll(const Vector3 &from, const Vector3 &dir, float length, int layer_mask = -1);
    };
}

#endif