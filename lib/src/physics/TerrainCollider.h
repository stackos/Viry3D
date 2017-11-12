#ifndef __TerrainCollider_h__
#define __TerrainCollider_h__

#include "Collider.h"
#include "Terrain.h"

class btTriangleIndexVertexArray;

namespace Galaxy3D
{
    class TerrainCollider : public Collider
    {
    public:
        TerrainCollider():
            m_collider_data(NULL)
        {}
        virtual ~TerrainCollider();
        void SetTerrain(const std::shared_ptr<Terrain> &terrain) {m_terrain = terrain;}

    protected:
        virtual void Start();

    private:
        std::shared_ptr<Terrain> m_terrain;
        btTriangleIndexVertexArray *m_collider_data;
    };
}

#endif