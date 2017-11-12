#include "NavMeshAgent.h"
#include "NavMesh.h"
#include "Transform.h"

namespace Galaxy3D
{
    void NavMeshAgent::Start()
    {
        SamplePosition();
    }

    void NavMeshAgent::SamplePosition()
    {
        Vector3 pos = GetTransform()->GetPosition();
        int tri_index = NavMesh::FindTriangle(pos);
        if(tri_index >= 0)
        {
            pos = NavMesh::GetPosition(tri_index, pos.x, pos.z);
            GetTransform()->SetPosition(pos);
            m_navmesh_triangle_index = tri_index;
        }
    }

    void NavMeshAgent::Move(const Vector3 &offset)
    {
        if(m_navmesh_triangle_index < 0)
        {
            return;
        }

        Vector3 pos;
        int result_index = NavMesh::Move(GetTransform()->GetPosition(), m_navmesh_triangle_index, offset, pos);
        
        if(result_index >= 0)
        {
            m_navmesh_triangle_index = result_index;
            GetTransform()->SetPosition(pos);
        }
    }
}