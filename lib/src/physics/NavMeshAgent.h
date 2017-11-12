#ifndef __NavMeshAgent_h__
#define __NavMeshAgent_h__

#include "Component.h"
#include "Vector3.h"

namespace Galaxy3D
{
    class NavMeshAgent : public Component
    {
    public:
        NavMeshAgent():
            m_navmesh_triangle_index(-1)
        {}
        void SamplePosition();
        void Move(const Vector3 &offset);

    protected:
        virtual void Start();

    private:
        int m_navmesh_triangle_index;
    };
}

#endif