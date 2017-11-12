#include "NavMesh.h"
#include "GTFile.h"
#include "Mathf.h"
#include <list>

namespace Galaxy3D
{
    std::vector<NavTriangle> NavMesh::m_triangles;
    std::vector<Vector3> NavMesh::m_vertices;
    std::vector<int> NavMesh::m_indices;

    void NavMesh::LoadFromFile(const std::string &file)
    {
        int file_size;
        char *bytes = (char *) GTFile::ReadAllBytes(file, &file_size);
        if(bytes != NULL)
        {
            char *p = bytes;

            int vertex_count;
            BUFFER_READ(vertex_count, p, 4);
            m_vertices.resize(vertex_count);
            BUFFER_READ(m_vertices[0], p, sizeof(Vector3) * vertex_count);

            int index_count;
            BUFFER_READ(index_count, p, 4);
            m_indices.resize(index_count);
            BUFFER_READ(m_indices[0], p, sizeof(int) * index_count);

            int triangle_count;
            BUFFER_READ(triangle_count, p, 4);
            m_triangles.resize(triangle_count);
            BUFFER_READ(m_triangles[0], p, sizeof(NavTriangle) * triangle_count);

            free(bytes);
        }
    }

    int NavMesh::FindTriangle(const Vector3 &pos)
    {
        int index = -1;

        for(size_t i=0; i<m_triangles.size(); i++)
        {
            if(IsInTriangle(pos, i))
            {
                index = i;
                break;
            }
        }

        return index;
    }

    Vector3 NavMesh::GetPosition(int index, float x, float z)
    {
        Vector3 v(x, 0, z);

        auto &t = m_triangles[index];
        
        Vector3 v0 = m_vertices[t.edges[0].vertex_left];
        Vector3 v1 = m_vertices[t.edges[1].vertex_left];
        Vector3 v2 = m_vertices[t.edges[2].vertex_left];

        float div_u = (v0.x - v2.x) * (v1.z - v2.z) - (v1.x - v2.x) * (v0.z - v2.z);
        float div_w = v1.x - v2.x;

        if(Mathf::FloatEqual(div_u, 0) || Mathf::FloatEqual(div_w, 0))
        {
            v0 = m_vertices[t.edges[1].vertex_left];
            v1 = m_vertices[t.edges[0].vertex_left];
        }

        float u = ((v1.x - v2.x) * (v2.z - v.z) - (v2.x - v.x) * (v1.z - v2.z)) /
            ((v0.x - v2.x) * (v1.z - v2.z) - (v1.x - v2.x) * (v0.z - v2.z));
        float w = ((v.x - v2.x) - (v0.x - v2.x) * u) / (v1.x - v2.x);

        v.y = v0.y * u + v1.y * w + (1 - u - w) * v2.y;

        return v;
    }

    struct AStarNode
    {
        int triangle_index;
        AStarNode *parent;
        float g;
        float h;

        AStarNode():
            triangle_index(-1),
            parent(NULL),
            g(0),
            h(0)
        {
        }

        static bool Less(const AStarNode *a, const AStarNode *b)
        {
            return a->g + a->h < b->g + b->h;
        }

        static AStarNode *FindInList(std::list<AStarNode *> &list, int triangle_index)
        {
            for(auto &i : list)
            {
                if(i->triangle_index == triangle_index)
                {
                    return i;
                }
            }

            return NULL;
        }
    };

    std::list<AStarNode *> open;
    std::list<AStarNode *> close;

    bool NavMesh::CalculatePath(const Vector3 &source, int source_triangle_index, const Vector3 &target, NavMeshPath &path)
    {
        Vector3 begin = GetPosition(source_triangle_index, source.x, source.z);

        bool in_same_node = IsInTriangle(target, source_triangle_index);
        if(in_same_node)
        {
            Vector3 end = GetPosition(source_triangle_index, target.x, target.z);

            path.corners.clear();
            path.corners.push_back(begin);
            path.corners.push_back(end);
            return true;
        }

        AStarNode *first = new AStarNode();
        first->triangle_index = source_triangle_index;
        open.push_back(first);

        AStarNode *found_node = NULL;

        while(!open.empty())
        {
            open.sort(AStarNode::Less);
            auto node = open.front();
            open.remove(node);
            close.push_back(node);

            if(IsInTriangle(target, node->triangle_index))
            {
                found_node = node;
                break;
            }

            for(int i=0; i<3; i++)
            {
                int neighbor = m_triangles[node->triangle_index].edges[i].neighbor;

                if(neighbor >= 0)
                {
                    if(AStarNode::FindInList(close, neighbor) == NULL)
                    {
                        AStarNode *neighbor_node = new AStarNode();
                        neighbor_node->triangle_index = neighbor;
                        neighbor_node->parent = node;

                        auto &neighbor_triangle = m_triangles[neighbor_node->triangle_index];
                        Vector3 v0 = m_vertices[neighbor_triangle.edges[0].vertex_left];
                        Vector3 v1 = m_vertices[neighbor_triangle.edges[1].vertex_left];
                        Vector3 v2 = m_vertices[neighbor_triangle.edges[2].vertex_left];
                        Vector3 center = (v0 + v1 + v2) * (1.0f / 3);
                        center.y = 0;
                        Vector3 end = target;
                        end.y = 0;
                        neighbor_node->h = (center - end).SqrMagnitude();

                        {
                            auto &parent_triangle = m_triangles[neighbor_node->parent->triangle_index];
                            Vector3 parent_v0 = m_vertices[parent_triangle.edges[0].vertex_left];
                            Vector3 parent_v1 = m_vertices[parent_triangle.edges[1].vertex_left];
                            Vector3 parent_v2 = m_vertices[parent_triangle.edges[2].vertex_left];
                            Vector3 parent_center = (parent_v0 + parent_v1 + parent_v2) * (1.0f / 3);
                            center = (v0 + v1 + v2) * (1.0f / 3);

                            auto &shared_edge = m_triangles[node->triangle_index].edges[i];
                            Vector3 edge_left = m_vertices[shared_edge.vertex_left];
                            Vector3 edge_right = m_vertices[shared_edge.vertex_right];
                            Vector3 edge_center = (edge_left + edge_right) * 0.5f;

                            neighbor_node->g = neighbor_node->parent->g + 
                                (parent_center - edge_center).SqrMagnitude() +
                                (center - edge_center).SqrMagnitude();
                        }

                        AStarNode *node_in_open = AStarNode::FindInList(open, neighbor);

                        if(node_in_open == NULL)
                        {
                            open.push_back(neighbor_node);
                        }
                        else
                        {
                            if(neighbor_node->g < node_in_open->g)
                            {
                                node_in_open->g = neighbor_node->g;
                                node_in_open->parent = node;
                            }

                            delete neighbor_node;
                        }
                    }
                }
            }
        }

        if(found_node != NULL)
        {
            //path.corners.clear();
            //path.corners.push_back(begin);
            //path.corners.push_back(end);

            int depth = 0;
            auto node = found_node;
            while(node->parent != NULL)
            {
                node = node->parent;
                depth++;
            }

            return true;
        }

        return false;
    }

    static float triangle_area2(const Vector3 &a, const Vector3 &b, const Vector3 &c)
    {
        return (a.x - c.x) * (b.z - c.z) - (a.z - c.z) * (b.x - c.x);
    }

    static bool line_intersect(const Vector3 &a_l, const Vector3 &a_r, const Vector3 &b_l, const Vector3 &b_r, Vector3 &intersect)
    {
        float sabc = triangle_area2(a_l, a_r, b_l);
        float sabd = triangle_area2(a_l, a_r, b_r);

        if(fabsf(sabc) < 0.0001f)
        {
            // c在ab上
            intersect.x = b_l.x;
            intersect.y = 0;
            intersect.z = b_l.z;

            return true;
        }
        else if(fabsf(sabd) < 0.0001f)
        {
            // d在ab上
            intersect.x = b_r.x;
            intersect.y = 0;
            intersect.z = b_r.z;

            return true;
        }

        if(sabc * sabd < 0)
        {
            // cd位于ab两端
            float inv = 1.0f / (sabd - sabc);
            float x = (sabd * b_l.x - sabc * b_r.x) * inv;
            float z = (sabd * b_l.z - sabc * b_r.z) * inv;
            intersect.x = x;
            intersect.y = 0;
            intersect.z = z;

            return true;
        }
        else
        {
            return false;
        }
    }

    bool NavMesh::IsInTriangle(const Vector3 &pos, int index)
    {
        Vector3 v = pos;
        v.y = 0;

        Vector3 v0 = m_vertices[m_triangles[index].edges[0].vertex_left];
        Vector3 v1 = m_vertices[m_triangles[index].edges[1].vertex_left];
        Vector3 v2 = m_vertices[m_triangles[index].edges[2].vertex_left];

        v0.y = 0;
        v1.y = 0;
        v2.y = 0;

        Vector3 c0 = (v1 - v0) * (v - v0);
        Vector3 c1 = (v2 - v1) * (v - v1);
        Vector3 c2 = (v0 - v2) * (v - v2);
        if( !(c0.y * c1.y < -Mathf::Epsilon ||
            c1.y * c2.y < -Mathf::Epsilon ||
            c2.y * c0.y < -Mathf::Epsilon))
        {
            return true;
        }

        return false;
    }

    int NavMesh::Move(const Vector3 &source, int source_triangle_index, const Vector3 &offset, Vector3 &out_pos)
    {
        int result_index = -1;

        Vector3 src = source;
        src.y = 0;
        Vector3 dir = offset;
        dir.y = 0;

        Vector3 src_next = src;
        Vector3 offset_next = dir;
        int triangle_next = source_triangle_index;

        int triangle_old;
        while(triangle_next >= 0)
        {
            triangle_old = triangle_next;

            Vector3 target = src_next + offset_next;
            bool in_node = IsInTriangle(target, triangle_next);
            if(in_node)
            {
                result_index = triangle_next;
                out_pos = GetPosition(triangle_next, target.x, target.z);
                return result_index;
            }

            Vector3 intersect[3] = {0};
            bool have_intersect[3] = {false};

            for(int i=0; i<3; i++)
            {
                auto &edge = m_triangles[triangle_next].edges[i];
                Vector3 left = m_vertices[edge.vertex_left];
                left.y = 0;
                Vector3 right = m_vertices[edge.vertex_right];
                right.y = 0;

                if(src_next == left || src_next == right)
                {
                    intersect[i] = src_next;
                    intersect[i].y = 0;
                    have_intersect[i] = true;
                }
                else
                {
                    bool result = line_intersect(left, right, src_next, target, intersect[i]);
                    have_intersect[i] = result;

                    if(result)
                    {
                        if(fabs(right.x - left.x) > fabs(right.z - left.z))
                        {
                            float t = (intersect[i].x - left.x) / (right.x - left.x);

                            if(t < -Mathf::Epsilon || t > 1 + Mathf::Epsilon)
                            {
                                have_intersect[i] = false;
                            }
                            else
                            {
                                intersect[i] = Vector3::Lerp(left, right, t);
                                intersect[i].y = 0;
                            }
                        }
                        else
                        {
                            float t = (intersect[i].z - left.z) / (right.z - left.z);

                            if(t < -Mathf::Epsilon || t > 1 + Mathf::Epsilon)
                            {
                                have_intersect[i] = false;
                            }
                            else
                            {
                                intersect[i] = Vector3::Lerp(left, right, t);
                                intersect[i].y = 0;
                            }
                        }
                    }
                }
            }

            for(int i=0; i<3; i++)
            {
                if(have_intersect[i])
                {
                    auto &edge = m_triangles[triangle_next].edges[i];
                    Vector3 left = m_vertices[edge.vertex_left];
                    left.y = 0;
                    Vector3 right = m_vertices[edge.vertex_right];
                    right.y = 0;
                    Vector3 edge_normal = Vector3(left.z - right.z, 0, right.x - left.x);
                    float dot = edge_normal.Dot(offset_next);

                    if(!(dot < 0))
                    {
                        if(edge.neighbor >= 0)
                        {
                            offset_next = offset_next - (intersect[i] - src_next);
                            src_next = intersect[i];
                            triangle_next = edge.neighbor;

                            break;
                        }
                        else
                        {
                            Vector3 edge_dir = left - right;
                            Vector3 offset_dir = offset_next - (intersect[i] - src_next);
                            Vector3 offset_in_edge = edge_dir * (offset_dir.Dot(edge_dir) / edge_dir.SqrMagnitude());

                            offset_next = offset_in_edge;
                            src_next = intersect[i];

                            target = src_next + offset_next;
                            if(fabs(right.x - left.x) > fabs(right.z - left.z))
                            {
                                float t = (target.x - left.x) / (right.x - left.x);
                                target = Vector3::Lerp(left, right, t);
                            }
                            else
                            {
                                float t = (target.z - left.z) / (right.z - left.z);
                                target = Vector3::Lerp(left, right, t);
                            }

                            // 注意：理论上target是在边上的，可能会由于精度问题，导致计算结果位于三角形外部
                            // 使Mathf::Epsilon = 0.00001f可以容纳这个精度误差

                            result_index = triangle_next;
                            out_pos = GetPosition(triangle_next, target.x, target.z);
                            return result_index;
                        }
                    }
                }
            }

            // will loop infinite.
            // for debug break
            if(triangle_old == triangle_next)
            {
                src_next = src;
                offset_next = dir;
                triangle_next = source_triangle_index;
            }
        }

        return result_index;
    }
}