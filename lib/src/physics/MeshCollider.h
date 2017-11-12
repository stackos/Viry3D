#ifndef __MeshCollider_h__
#define __MeshCollider_h__

#include "Collider.h"
#include "Mesh.h"

class btTriangleIndexVertexArray;

namespace Galaxy3D
{
    class MeshCollider : public Collider
    {
    public:
        MeshCollider():
            m_collider_data(NULL),
            m_indices(NULL),
            m_vertices(NULL)
        {}
        virtual ~MeshCollider();
        void SetMesh(const std::shared_ptr<Mesh> &mesh) {m_mesh = mesh;}

    protected:
        virtual void Start();
        virtual void OnTranformChanged();

    private:
        std::shared_ptr<Mesh> m_mesh;
        btTriangleIndexVertexArray *m_collider_data;
        unsigned short *m_indices;
        unsigned char *m_vertices;
    };
}

#endif