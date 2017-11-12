#ifndef __CharacterController_h__
#define __CharacterController_h__

#include "Component.h"
#include "Vector3.h"

namespace Galaxy3D
{
    class CharacterController : public Component
    {
    public:
        CharacterController():
            m_center(0, 1, 0),
            m_height(2),
            m_radius(0.5f),
            m_step_height(0.3f),
            m_character(NULL),
            m_in_world(false)
        {
        }
        virtual ~CharacterController();
        void Move(const Vector3 &offset);

    protected:
        virtual void Start();
        virtual void Update();
        virtual void OnTranformChanged();
        virtual void OnEnable();
        virtual void OnDisable();

    private:
        Vector3 m_center;
        float m_height;
        float m_radius;
        float m_step_height;
        void *m_character;
        bool m_in_world;
    };
}

#endif