#ifndef __NavMesh_h__
#define __NavMesh_h__

#include "Vector3.h"
#include <string>
#include <vector>

namespace Galaxy3D
{
    struct NavEdge
    {
        int vertex_left;
        int vertex_right;
        int neighbor;

        NavEdge(){}

        NavEdge(int left, int right)
        {
            vertex_left = left;
            vertex_right = right;
            neighbor = -1;
        }
    };

    struct NavTriangle
    {
        NavEdge edges[3];
    };

    struct NavMeshPath
    {
        std::vector<Vector3> corners;
    };

    class NavMesh
    {
    public:
        static void LoadFromFile(const std::string &file);
        // 根据x, y找到navmesh内对应三角形, 返回三角形索引
        static int FindTriangle(const Vector3 &pos);
        static bool IsInTriangle(const Vector3 &pos, int index);
        static Vector3 GetPosition(int index, float x, float z);
        static bool CalculatePath(const Vector3 &source, int source_triangle_index, const Vector3 &target, NavMeshPath &path);
        static int Move(const Vector3 &source, int source_triangle_index, const Vector3 &offset, Vector3 &out_pos);

    private:
        static std::vector<NavTriangle> m_triangles;
        static std::vector<Vector3> m_vertices;
        static std::vector<int> m_indices;
    };
}

#endif