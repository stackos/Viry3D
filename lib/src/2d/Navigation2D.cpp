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

#include "Navigation2D.h"

namespace Viry3D
{
    Navigation2D::Navigation2D():
        m_cell_size(1), // one pixel
        m_last_id(1)
    {

    }

    int Navigation2D::NavpolyAdd(const std::shared_ptr<NavigationPolygon>& p_mesh, const Transform2D& p_xform, Node* owner)
    {
        int id = m_last_id++;
        NavMesh nm;
        nm.linked = false;
        nm.navpoly = p_mesh;
        nm.xform = p_xform;
        nm.owner = owner;
        m_navpoly_map[id] = nm;

        this->NavpolyLink(id);

        return id;
    }

    void Navigation2D::NavpolyLink(int p_id)
    {
        NavMesh& nm = m_navpoly_map[p_id];

        auto& vertices = nm.navpoly->GetVertices();
        int len = (int) vertices.size();
        if (len == 0)
            return;

        for (int i = 0; i < nm.navpoly->GetPolygonCount(); i++)
        {
            // build

            nm.polygons.push_back(Polygon());
            Polygon& p = nm.polygons.back();
            p.owner = &nm;

            auto& poly = nm.navpoly->GetPolygon(i);
            int plen = (int) poly.size();
            const int* indices = poly.data();
            bool valid = true;
            p.edges.resize(plen);

            Vector2 center;
            float sum = 0;

            for (int j = 0; j < plen; j++)
            {
                int idx = indices[j];
                if (idx < 0 || idx >= len)
                {
                    valid = false;
                    break;
                }

                Polygon::Edge e;
                Vector2 ep = nm.xform.xform(vertices[idx]);
                center += ep;
                e.point = this->GetPoint(ep);
                p.edges[j] = e;

                int idxn = indices[(j + 1) % plen];
                if (idxn < 0 || idxn >= len)
                {
                    valid = false;
                    break;
                }

                Vector2 epn = nm.xform.xform(vertices[idxn]);

                sum += (epn.x - ep.x) * (epn.y + ep.y);
            }

            p.clockwise = sum > 0;

            if (!valid)
            {
                nm.polygons.pop_back();
                continue;
            }

            p.center = center / (float) plen;

            // connect

            for (int j = 0; j < plen; j++)
            {
                int next = (j + 1) % plen;
                EdgeKey ek(p.edges[j].point, p.edges[next].point);

                auto C = m_connections.find(ek);
                if (C == m_connections.end())
                {
                    Connection c;
                    c.A = &p;
                    c.A_edge = j;
                    c.B = nullptr;
                    c.B_edge = -1;
                    m_connections[ek] = c;
                }
                else
                {
                    if (C->second.B != nullptr)
                    {
                        ConnectionPending pending;
                        pending.polygon = &p;
                        pending.edge = j;
                        C->second.pending.push_back(pending);
                        p.edges[j].P = &C->second.pending.back();
                        continue;
                    }

                    C->second.B = &p;
                    C->second.B_edge = j;
                    C->second.A->edges[C->second.A_edge].C = &p;
                    C->second.A->edges[C->second.A_edge].C_edge = j;
                    p.edges[j].C = C->second.A;
                    p.edges[j].C_edge = C->second.A_edge;
                    // connection successful.
                }
            }
        }

        nm.linked = true;
    }

    void Navigation2D::NavpolyUnlink(int p_id)
    {
        NavMesh& nm = m_navpoly_map[p_id];

        for (auto E = nm.polygons.begin(); E != nm.polygons.end(); ++E)
        {
            Polygon& p = *E;

            int ec = (int) p.edges.size();
            Polygon::Edge* edges = p.edges.data();

            for (int i = 0; i < ec; i++)
            {
                int next = (i + 1) % ec;

                EdgeKey ek(edges[i].point, edges[next].point);
                auto C = m_connections.find(ek);

                if (edges[i].P)
                {
                    C->second.pending.remove_if([&](const ConnectionPending& ele) {
                        return edges[i].P == &ele;
                    });
                    edges[i].P = nullptr;
                }
                else if (C->second.B)
                {
                    // disconnect

                    C->second.B->edges[C->second.B_edge].C = nullptr;
                    C->second.B->edges[C->second.B_edge].C_edge = -1;
                    C->second.A->edges[C->second.A_edge].C = nullptr;
                    C->second.A->edges[C->second.A_edge].C_edge = -1;

                    if (C->second.A == &*E)
                    {
                        C->second.A = C->second.B;
                        C->second.A_edge = C->second.B_edge;
                    }
                    C->second.B = nullptr;
                    C->second.B_edge = -1;

                    if (C->second.pending.size())
                    {
                        // reconnect if something is pending
                        ConnectionPending cp = C->second.pending.front();
                        C->second.pending.pop_front();

                        C->second.B = cp.polygon;
                        C->second.B_edge = cp.edge;
                        C->second.A->edges[C->second.A_edge].C = cp.polygon;
                        C->second.A->edges[C->second.A_edge].C_edge = cp.edge;
                        cp.polygon->edges[cp.edge].C = C->second.A;
                        cp.polygon->edges[cp.edge].C_edge = C->second.A_edge;
                        cp.polygon->edges[cp.edge].P = nullptr;
                    }
                }
                else
                {
                    m_connections.erase(C);
                    // erase
                }
            }
        }

        nm.polygons.clear();

        nm.linked = false;
    }

    void Navigation2D::NavpolySetTransform(int p_id, const Transform2D& p_xform)
    {
        NavMesh& nm = m_navpoly_map[p_id];
        if (nm.xform == p_xform)
            return; // bleh
        this->NavpolyUnlink(p_id);
        nm.xform = p_xform;
        this->NavpolyLink(p_id);
    }

    void Navigation2D::NavpolyRemove(int p_id)
    {
        this->NavpolyUnlink(p_id);
        m_navpoly_map.erase(p_id);
    }

    Vector2 Navigation2D::GetClosestPoint(const Vector2& p_point)
    {
        Vector2 closest_point = Vector2();

        return closest_point;
    }

    Node* Navigation2D::GetClosestPointOwner(const Vector2& p_point)
    {
        Node* owner = nullptr;

        return owner;
    }

    std::vector<Vector2> Navigation2D::GetSimplePath(const Vector2& p_start, const Vector2& p_end, bool p_optimize)
    {
        return std::vector<Vector2>();
    }
}
