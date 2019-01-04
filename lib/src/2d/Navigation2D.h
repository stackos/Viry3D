/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "math/Vector2.h"
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <cmath>

namespace Viry3D
{
    class NavigationPolygon;

    struct Transform2D
    {
        Vector2 elements[3];

        inline float tdotx(const Vector2& v) const { return elements[0].x * v.x + elements[1].x * v.y; }
        inline float tdoty(const Vector2& v) const { return elements[0].y * v.x + elements[1].y * v.y; }
        inline Vector2 xform(const Vector2& p_vec) const
        {
            return Vector2(tdotx(p_vec), tdoty(p_vec)) + elements[2];
        }

        bool operator ==(const Transform2D& p_transform) const
        {
            for (int i = 0; i < 3; i++)
            {
                if (elements[i] != p_transform.elements[i])
                    return false;
            }

            return true;
        }

        Transform2D()
        {
            elements[0] = Vector2(1.0f, 0.0f);
            elements[1] = Vector2(0.0f, 1.0f);
            elements[2] = Vector2(0.0f, 0.0f);
        }

        Transform2D(float p_rot, const Vector2& p_pos)
        {
            float cr = cos(p_rot);
            float sr = sin(p_rot);
            elements[0].x = cr;
            elements[0].y = sr;
            elements[1].x = -sr;
            elements[1].y = cr;
            elements[2] = p_pos;
        }
    };

    class Navigation2D
    {
        union Point
        {
            struct
            {
                int64_t x : 32;
                int64_t y : 32;
            };

            uint64_t key;

            bool operator <(const Point& p_key) const { return key < p_key.key; }
        };

        struct EdgeKey
        {
            Point a;
            Point b;

            bool operator <(const EdgeKey& p_key) const
            {
                return (a.key == p_key.a.key) ? (b.key < p_key.b.key) : (a.key < p_key.a.key);
            }

            EdgeKey(const Point& p_a = Point(), const Point& p_b = Point()):
                a(p_a),
                b(p_b)
            {
                if (a.key > b.key)
                {
                    std::swap(a, b);
                }
            }
        };

        struct Polygon;
        struct NavMesh;

        struct ConnectionPending
        {
            Polygon* polygon;
            int edge;
        };

        struct Polygon
        {
            struct Edge
            {
                Point point;
                Polygon* C;
                int C_edge;
                ConnectionPending* P;

                Edge()
                {
                    C = nullptr;
                    C_edge = -1;
                    P = nullptr;
                }
            };

            std::vector<Edge> edges;
            Vector2 center;
            Vector2 entry;
            float distance;
            int prev_edge;
            bool clockwise;
            NavMesh* owner;
        };

        struct Connection
        {
            Polygon* A;
            int A_edge;
            Polygon* B;
            int B_edge;
            std::list<ConnectionPending> pending;

            Connection()
            {
                A = nullptr;
                B = nullptr;
                A_edge = -1;
                B_edge = -1;
            }
        };

        struct NavMesh
        {
            void* owner;
            Transform2D xform;
            bool linked;
            std::shared_ptr<NavigationPolygon> navpoly;
            std::list<Polygon> polygons;
        };

    public:
        Navigation2D();
        int NavpolyAdd(const std::shared_ptr<NavigationPolygon>& p_mesh, const Transform2D& p_xform, void* p_owner = nullptr);
        void NavpolySetTransform(int p_id, const Transform2D& p_xform);
        void NavpolyRemove(int p_id);
        std::vector<Vector2> GetSimplePath(const Vector2& p_start, const Vector2& p_end, bool p_optimize = true);
        Vector2 GetClosestPoint(const Vector2& p_point);
        void* GetClosestPointOwner(const Vector2& p_point);

    private:
        inline Point GetPoint(const Vector2& p_pos) const
        {
            int x = int(floor(p_pos.x / m_cell_size));
            int y = int(floor(p_pos.y / m_cell_size));

            Point p;
            p.key = 0;
            p.x = x;
            p.y = y;
            return p;
        }

        inline Vector2 GetVertex(const Point& p_point) const
        {
            return Vector2((float) p_point.x, (float) p_point.y) * m_cell_size;
        }

        void NavpolyLink(int p_id);
        void NavpolyUnlink(int p_id);

    private:
        std::map<EdgeKey, Connection> m_connections;
        float m_cell_size;
        std::map<int, NavMesh> m_navpoly_map;
        int m_last_id;
    };
}
