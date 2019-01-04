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
#include "NavigationPolygon.h"
#include <algorithm>

#define USE_ENTRY_POINT

namespace Viry3D
{
    Navigation2D::Navigation2D():
        m_cell_size(1), // one pixel
        m_last_id(1)
    {

    }

    int Navigation2D::NavpolyAdd(const std::shared_ptr<NavigationPolygon>& p_mesh, const Transform2D& p_xform, void* owner)
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

            p.center = center * (1.0f / (float) plen);

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

    static Vector2 get_closest_point_to_segment_2d(const Vector2& p_point, const Vector2* p_segment)
    {
        Vector2 p = p_point - p_segment[0];
        Vector2 n = p_segment[1] - p_segment[0];
        float l = n.Magnitude();
        if (l < 1e-10f)
            return p_segment[0]; // both points are the same, just give any
        n *= 1.0f / l;

        float d = n.Dot(p);

        if (d <= 0.0f)
            return p_segment[0]; // before first point
        else if (d >= l)
            return p_segment[1]; // after first point
        else
            return p_segment[0] + n * d; // inside
    }

    Vector2 Navigation2D::GetClosestPoint(const Vector2& p_point)
    {
        Vector2 closest_point = Vector2();
        float closest_point_d = 1e20f;

        for (auto& E : m_navpoly_map)
        {
            if (!E.second.linked)
                continue;
            for (auto& F : E.second.polygons)
            {
                Polygon& p = F;
                for (int i = 2; i < (int) p.edges.size(); i++)
                {
                    if (is_point_in_triangle(p_point, this->GetVertex(p.edges[0].point), this->GetVertex(p.edges[i - 1].point), this->GetVertex(p.edges[i].point)))
                    {
                        return p_point; //inside triangle, nothing else to discuss
                    }
                }
            }
        }

        for (auto& E : m_navpoly_map)
        {
            if (!E.second.linked)
                continue;
            for (auto& F : E.second.polygons)
            {
                Polygon& p = F;
                int es = (int) p.edges.size();
                for (int i = 0; i < es; i++)
                {
                    Vector2 edge[2] = {
                        this->GetVertex(p.edges[i].point),
                        this->GetVertex(p.edges[(i + 1) % es].point)
                    };

                    Vector2 spoint = get_closest_point_to_segment_2d(p_point, edge);
                    float d = (spoint - p_point).SqrMagnitude();
                    if (d < closest_point_d)
                    {
                        closest_point = spoint;
                        closest_point_d = d;
                    }
                }
            }
        }

        return closest_point;
    }

    void* Navigation2D::GetClosestPointOwner(const Vector2& p_point)
    {
        void* owner = nullptr;
        Vector2 closest_point = Vector2();
        float closest_point_d = 1e20f;

        for (auto& E : m_navpoly_map)
        {
            if (!E.second.linked)
                continue;
            for (auto& F : E.second.polygons)
            {
                Polygon& p = F;
                for (int i = 2; i < (int) p.edges.size(); i++)
                {
                    if (is_point_in_triangle(p_point, this->GetVertex(p.edges[0].point), this->GetVertex(p.edges[i - 1].point), this->GetVertex(p.edges[i].point)))
                    {
                        return E.second.owner;
                    }
                }
            }
        }

        for (auto& E : m_navpoly_map)
        {
            if (!E.second.linked)
                continue;
            for (auto& F : E.second.polygons)
            {
                Polygon& p = F;
                int es = (int) p.edges.size();
                for (int i = 0; i < es; i++)
                {
                    Vector2 edge[2] = {
                        this->GetVertex(p.edges[i].point),
                        this->GetVertex(p.edges[(i + 1) % es].point)
                    };

                    Vector2 spoint = get_closest_point_to_segment_2d(p_point, edge);
                    float d = (spoint - p_point).SqrMagnitude();
                    if (d < closest_point_d)
                    {
                        closest_point = spoint;
                        closest_point_d = d;
                        owner = E.second.owner;
                    }
                }
            }
        }

        return owner;
    }

    std::vector<Vector2> Navigation2D::GetSimplePath(const Vector2& p_start, const Vector2& p_end, bool p_optimize)
    {
        Polygon* begin_poly = nullptr;
        Polygon* end_poly = nullptr;
        Vector2 begin_point;
        Vector2 end_point;
        float begin_d = 1e20f;
        float end_d = 1e20f;

        //look for point inside triangle

        for (auto& E : m_navpoly_map)
        {
            if (!E.second.linked)
                continue;
            for (auto& F : E.second.polygons)
            {
                Polygon& p = F;
                if (begin_d || end_d)
                {
                    for (int i = 2; i < (int) p.edges.size(); i++)
                    {
                        if (begin_d > 0)
                        {
                            if (is_point_in_triangle(p_start, this->GetVertex(p.edges[0].point), this->GetVertex(p.edges[i - 1].point), this->GetVertex(p.edges[i].point)))
                            {
                                begin_poly = &p;
                                begin_point = p_start;
                                begin_d = 0;
                                if (end_d == 0)
                                    break;
                            }
                        }

                        if (end_d > 0)
                        {
                            if (is_point_in_triangle(p_end, this->GetVertex(p.edges[0].point), this->GetVertex(p.edges[i - 1].point), this->GetVertex(p.edges[i].point)))
                            {
                                end_poly = &p;
                                end_point = p_end;
                                end_d = 0;
                                if (begin_d == 0)
                                    break;
                            }
                        }
                    }
                }

                p.prev_edge = -1;
            }
        }

        //start or end not inside triangle.. look for closest segment :|
        if (begin_d || end_d)
        {
            for (auto& E : m_navpoly_map)
            {
                if (!E.second.linked)
                    continue;
                for (auto& F : E.second.polygons)
                {
                    Polygon& p = F;
                    int es = (int) p.edges.size();
                    for (int i = 0; i < es; i++)
                    {
                        Vector2 edge[2] = {
                            this->GetVertex(p.edges[i].point),
                            this->GetVertex(p.edges[(i + 1) % es].point)
                        };

                        if (begin_d > 0)
                        {
                            Vector2 spoint = get_closest_point_to_segment_2d(p_start, edge);
                            float d = (spoint - p_start).SqrMagnitude();
                            if (d < begin_d)
                            {
                                begin_poly = &p;
                                begin_point = spoint;
                                begin_d = d;
                            }
                        }

                        if (end_d > 0)
                        {
                            Vector2 spoint = get_closest_point_to_segment_2d(p_end, edge);
                            float d = (spoint - p_end).SqrMagnitude();
                            if (d < end_d)
                            {
                                end_poly = &p;
                                end_point = spoint;
                                end_d = d;
                            }
                        }
                    }
                }
            }
        }

        if (!begin_poly || !end_poly)
        {
            return std::vector<Vector2>(); //no path
        }

        if (begin_poly == end_poly)
        {
            std::vector<Vector2> path;
            path.resize(2);
            path[0] = begin_point;
            path[1] = end_point;
            return path;
        }

        bool found_route = false;

        std::list<Polygon*> open_list;

        begin_poly->entry = p_start;

        for (int i = 0; i < (int) begin_poly->edges.size(); i++)
        {
            if (begin_poly->edges[i].C)
            {
                begin_poly->edges[i].C->prev_edge = begin_poly->edges[i].C_edge;
#ifdef USE_ENTRY_POINT
                Vector2 edge[2] = {
                    this->GetVertex(begin_poly->edges[i].point),
                    this->GetVertex(begin_poly->edges[(i + 1) % begin_poly->edges.size()].point)
                };

                Vector2 entry = get_closest_point_to_segment_2d(begin_poly->entry, edge);
                begin_poly->edges[i].C->distance = (begin_poly->entry - entry).Magnitude();
                begin_poly->edges[i].C->entry = entry;
#else
                begin_poly->edges[i].C->distance = (begin_poly->center - begin_poly->edges[i].C->center).Magnitude();
#endif
                open_list.push_back(begin_poly->edges[i].C);

                if (begin_poly->edges[i].C == end_poly)
                {
                    found_route = true;
                }
            }
        }

        while (!found_route)
        {
            if (open_list.size() == 0)
            {
                break;
            }
            //check open list

            Polygon* least_cost_poly = nullptr;
            float least_cost = 1e30f;

            //this could be faster (cache previous results)
            for (auto& E : open_list)
            {
                Polygon* p = E;

                float cost = p->distance;

#ifdef USE_ENTRY_POINT
                int es = (int) p->edges.size();

                float shortest_distance = 1e30f;

                for (int i = 0; i < es; i++)
                {
                    Polygon::Edge& e = p->edges[i];

                    if (!e.C)
                        continue;

                    Vector2 edge[2] = {
                        this->GetVertex(p->edges[i].point),
                        this->GetVertex(p->edges[(i + 1) % es].point)
                    };

                    Vector2 edge_point = get_closest_point_to_segment_2d(p->entry, edge);
                    float dist = (p->entry - edge_point).Magnitude();
                    if (dist < shortest_distance)
                        shortest_distance = dist;
                }

                cost += shortest_distance;
#else
                cost += (p->center - end_point).Magnitude();
#endif
                if (cost < least_cost)
                {
                    least_cost_poly = E;
                    least_cost = cost;
                }
            }

            Polygon* p = least_cost_poly;
            //open the neighbours for search
            int es = (int) p->edges.size();

            for (int i = 0; i < es; i++)
            {
                Polygon::Edge& e = p->edges[i];

                if (!e.C)
                    continue;

#ifdef USE_ENTRY_POINT
                Vector2 edge[2] = {
                    this->GetVertex(p->edges[i].point),
                    this->GetVertex(p->edges[(i + 1) % es].point)
                };

                Vector2 edge_entry = get_closest_point_to_segment_2d(p->entry, edge);
                float distance = (p->entry - edge_entry).Magnitude() + p->distance;
#else
                float distance = (p->center - e.C->center).Magnitude() + p->distance;
#endif

                if (e.C->prev_edge != -1)
                {
                    //oh this was visited already, can we win the cost?

                    if (e.C->distance > distance)
                    {
                        e.C->prev_edge = e.C_edge;
                        e.C->distance = distance;
#ifdef USE_ENTRY_POINT
                        e.C->entry = edge_entry;
#endif
                    }
                }
                else
                {
                    //add to open neighbours

                    e.C->prev_edge = e.C_edge;
                    e.C->distance = distance;
#ifdef USE_ENTRY_POINT
                    e.C->entry = edge_entry;
#endif

                    open_list.push_back(e.C);

                    if (e.C == end_poly)
                    {
                        //oh my reached end! stop algorithm
                        found_route = true;
                        break;
                    }
                }
            }

            if (found_route)
                break;

            open_list.remove(least_cost_poly);
        }

        if (found_route)
        {
            std::vector<Vector2> path;

            if (p_optimize)
            {
                //string pulling

                Vector2 apex_point = end_point;
                Vector2 portal_left = apex_point;
                Vector2 portal_right = apex_point;
                Polygon* left_poly = end_poly;
                Polygon* right_poly = end_poly;
                Polygon* p = end_poly;

                while (p)
                {
                    Vector2 left;
                    Vector2 right;

#define CLOCK_TANGENT(m_a, m_b, m_c) ((((m_a).x - (m_c).x) * ((m_b).y - (m_c).y) - ((m_b).x - (m_c).x) * ((m_a).y - (m_c).y)))

                    if (p == begin_poly)
                    {
                        left = begin_point;
                        right = begin_point;
                    }
                    else
                    {
                        int prev = p->prev_edge;
                        int prev_n = (p->prev_edge + 1) % p->edges.size();
                        left = this->GetVertex(p->edges[prev].point);
                        right = this->GetVertex(p->edges[prev_n].point);

                        if (p->clockwise)
                        {
                            std::swap(left, right);
                        }
                    }

                    bool skip = false;

                    if (CLOCK_TANGENT(apex_point, portal_left, left) >= 0)
                    {
                        //process
                        if ((portal_left - apex_point).Magnitude() < CMP_EPSILON || CLOCK_TANGENT(apex_point, left, portal_right) > 0)
                        {
                            left_poly = p;
                            portal_left = left;
                        }
                        else
                        {
                            apex_point = portal_right;
                            p = right_poly;
                            left_poly = p;
                            portal_left = apex_point;
                            portal_right = apex_point;
                            if (!path.size() || (path[path.size() - 1] - apex_point).Magnitude() > CMP_EPSILON)
                                path.push_back(apex_point);
                            skip = true;
                        }
                    }

                    if (!skip && CLOCK_TANGENT(apex_point, portal_right, right) <= 0)
                    {
                        //process
                        if ((portal_right - apex_point).Magnitude() < CMP_EPSILON || CLOCK_TANGENT(apex_point, right, portal_left) < 0)
                        {
                            right_poly = p;
                            portal_right = right;
                        }
                        else
                        {
                            apex_point = portal_left;
                            p = left_poly;
                            right_poly = p;
                            portal_right = apex_point;
                            portal_left = apex_point;
                            if (!path.size() || (path[path.size() - 1] - apex_point).Magnitude() > CMP_EPSILON)
                                path.push_back(apex_point);
                        }
                    }

                    if (p != begin_poly)
                        p = p->edges[p->prev_edge].C;
                    else
                        p = nullptr;
                }

            }
            else
            {
                //midpoints
                Polygon* p = end_poly;

                while (true)
                {
                    int prev = p->prev_edge;
                    int prev_n = (p->prev_edge + 1) % p->edges.size();
                    Vector2 point = (this->GetVertex(p->edges[prev].point) + this->GetVertex(p->edges[prev_n].point)) * 0.5;
                    path.push_back(point);
                    p = p->edges[prev].C;
                    if (p == begin_poly)
                        break;
                }
            }

            if (!path.size() || (path[path.size() - 1] - begin_point).Magnitude() > CMP_EPSILON)
            {
                path.push_back(begin_point); // Add the begin point
            }
            else
            {
                path[path.size() - 1] = begin_point; // Replace first midpoint by the exact begin point
            }

            std::reverse(path.begin(), path.end());

            if (path.size() <= 1 || (path[path.size() - 1] - end_point).Magnitude() > CMP_EPSILON)
            {
                path.push_back(end_point); // Add the end point
            }
            else
            {
                path[path.size() - 1] = end_point; // Replace last midpoint by the exact end point
            }

            return path;
        }

        return std::vector<Vector2>();
    }
}
