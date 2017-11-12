#ifndef __Collider_h__
#define __Collider_h__

#include "Component.h"

namespace Galaxy3D
{
    class Collider : public Component
    {
    protected:
        void *m_rigidbody;
        bool m_in_world;

        Collider():
            m_rigidbody(NULL),
            m_in_world(false)
        {}
        virtual ~Collider();
        virtual void OnEnable();
        virtual void OnDisable();
    };
}

#endif