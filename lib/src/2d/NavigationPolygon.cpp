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

#include "NavigationPolygon.h"
#include "Navigation2D.h"
#include "Debug.h"
#include "math/Mathf.h"
#include <list>
#include <map>
#include <algorithm>

namespace Viry3D
{
    bool is_point_in_triangle(const Vector2& s, const Vector2& a, const Vector2& b, const Vector2& c)
    {
        Vector2 an = a - s;
        Vector2 bn = b - s;
        Vector2 cn = c - s;

        bool orientation = an * bn > 0;
        if ((bn * cn > 0) != orientation) return false;
        return (cn * an > 0) == orientation;
    }

    static float get_area(const std::vector<Vector2>& contour)
    {
        int n = (int) contour.size();
        const Vector2* c = &contour[0];

        float A = 0.0f;

        for (int p = n - 1, q = 0; q < n; p = q++)
        {
            A += c[p] * c[q];
        }
        return A * 0.5f;
    }

    static bool is_inside_triangle(float Ax, float Ay,
        float Bx, float By,
        float Cx, float Cy,
        float Px, float Py,
        bool include_edges)
    {
        float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
        float cCROSSap, bCROSScp, aCROSSbp;

        ax = Cx - Bx;
        ay = Cy - By;
        bx = Ax - Cx;
        by = Ay - Cy;
        cx = Bx - Ax;
        cy = By - Ay;
        apx = Px - Ax;
        apy = Py - Ay;
        bpx = Px - Bx;
        bpy = Py - By;
        cpx = Px - Cx;
        cpy = Py - Cy;

        aCROSSbp = ax * bpy - ay * bpx;
        cCROSSap = cx * apy - cy * apx;
        bCROSScp = bx * cpy - by * cpx;

        if (include_edges)
        {
            return ((aCROSSbp > 0.0f) && (bCROSScp > 0.0f) && (cCROSSap > 0.0f));
        }
        else
        {
            return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
        }
    }

    static bool segment_intersects_segment_2d(const Vector2& p_from_a, const Vector2& p_to_a, const Vector2& p_from_b, const Vector2& p_to_b, Vector2* r_result)
    {
        Vector2 B = p_to_a - p_from_a;
        Vector2 C = p_from_b - p_from_a;
        Vector2 D = p_to_b - p_from_a;

        float ABlen = B.Dot(B);
        if (ABlen <= 0)
            return false;
        Vector2 Bn = B * (1.0f / ABlen);
        C = Vector2(C.x * Bn.x + C.y * Bn.y, C.y * Bn.x - C.x * Bn.y);
        D = Vector2(D.x * Bn.x + D.y * Bn.y, D.y * Bn.x - D.x * Bn.y);

        if ((C.y < 0 && D.y < 0) || (C.y >= 0 && D.y >= 0))
            return false;

        float ABpos = D.x + (C.x - D.x) * D.y / (D.y - C.y);

        //  Fail if segment C-D crosses line A-B outside of segment A-B.
        if (ABpos < 0 || ABpos > 1.0)
            return false;

        //  (4) Apply the discovered position to line A-B in the original coordinate system.
        if (r_result)
            *r_result = p_from_a + B * ABpos;

        return true;
    }

    static bool snip(const std::vector<Vector2>& p_contour, int u, int v, int w, int n, const std::vector<int>& V, bool relaxed)
    {
        int p;
        float Ax, Ay, Bx, By, Cx, Cy, Px, Py;
        const Vector2* contour = &p_contour[0];

        Ax = contour[V[u]].x;
        Ay = contour[V[u]].y;

        Bx = contour[V[v]].x;
        By = contour[V[v]].y;

        Cx = contour[V[w]].x;
        Cy = contour[V[w]].y;

        // It can happen that the triangulation ends up with three aligned vertices to deal with.
        // In this scenario, making the check below strict may reject the possibility of
        // forming a last triangle with these aligned vertices, preventing the triangulatiom
        // from completing.
        // To avoid that we allow zero-area triangles if all else failed.
        float threshold = relaxed ? -CMP_EPSILON : CMP_EPSILON;

        if (threshold > (((Bx - Ax) * (Cy - Ay)) - ((By - Ay) * (Cx - Ax)))) return false;

        for (p = 0; p < n; p++)
        {
            if ((p == u) || (p == v) || (p == w)) continue;
            Px = contour[V[p]].x;
            Py = contour[V[p]].y;
            if (is_inside_triangle(Ax, Ay, Bx, By, Cx, Cy, Px, Py, relaxed)) return false;
        }

        return true;
    }

    static bool triangulate(const std::vector<Vector2>& contour, std::vector<int>& result)
    {
        /* allocate and initialize list of Vertices in polygon */

        int n = (int) contour.size();
        if (n < 3) return false;

        std::vector<int> V;
        V.resize(n);

        /* we want a counter-clockwise polygon in V */

        if (0.0f < get_area(contour))
            for (int v = 0; v < n; v++)
                V[v] = v;
        else
            for (int v = 0; v < n; v++)
                V[v] = (n - 1) - v;

        bool relaxed = false;

        int nv = n;

        /*  remove nv-2 Vertices, creating 1 triangle every time */
        int count = 2 * nv; /* error detection */

        for (int v = nv - 1; nv > 2;)
        {
            /* if we loop, it is probably a non-simple polygon */
            if (0 >= (count--))
            {
                if (relaxed)
                {
                    //** Triangulate: ERROR - probable bad polygon!
                    return false;
                }
                else
                {
                    // There may be aligned vertices that the strict
                    // checks prevent from triangulating. In this situation
                    // we are better off adding flat triangles than
                    // failing, so we relax the checks and try one last
                    // round.
                    // Only relaxing the constraints as a last resort avoids
                    // degenerate triangles when they aren't necessary.
                    count = 2 * nv;
                    relaxed = true;
                }
            }

            /* three consecutive vertices in current polygon, <u,v,w> */
            int u = v;
            if (nv <= u) u = 0; /* previous */
            v = u + 1;
            if (nv <= v) v = 0; /* new v    */
            int w = v + 1;
            if (nv <= w) w = 0; /* next     */

            if (snip(contour, u, v, w, nv, V, relaxed))
            {
                int a, b, c, s, t;

                /* true names of the vertices */
                a = V[u];
                b = V[v];
                c = V[w];

                /* output Triangle */
                result.push_back(a);
                result.push_back(b);
                result.push_back(c);

                /* remove v from remaining polygon */
                for (s = v, t = v + 1; t < nv; s++, t++)
                    V[s] = V[t];

                nv--;

                /* reset error detection counter */
                count = 2 * nv;
            }
        }

        return true;
    }

    static std::vector<int> triangulate_polygon(const std::vector<Vector2>& p_polygon)
    {
        std::vector<int> triangles;
        if (!triangulate(p_polygon, triangles))
            return std::vector<int>(); //fail
        return triangles;
    }

    static bool is_point_in_polygon(const Vector2& p_point, const std::vector<Vector2>& p_polygon)
    {
        std::vector<int> indices = triangulate_polygon(p_polygon);
        for (int j = 0; j + 3 <= (int) indices.size(); j += 3)
        {
            int i1 = indices[j], i2 = indices[j + 1], i3 = indices[j + 2];
            if (is_point_in_triangle(p_point, p_polygon[i1], p_polygon[i2], p_polygon[i3]))
                return true;
        }
        return false;
    }

    NavigationPolygon::NavigationPolygon():
        m_rect_cache_dirty(true)
    {

    }

    void NavigationPolygon::AddOutline(const std::vector<Vector2>& p_outline)
    {
        m_outlines.push_back(p_outline);
        m_rect_cache_dirty = true;
    }

    void NavigationPolygon::AddOutlineAtIndex(const std::vector<Vector2>& p_outline, int p_index)
    {
        m_outlines.insert(m_outlines.begin() + p_index, p_outline);
        m_rect_cache_dirty = true;
    }

    void NavigationPolygon::SetOutline(int p_idx, const std::vector<Vector2>& p_outline)
    {
        m_outlines[p_idx] = p_outline;
        m_rect_cache_dirty = true;
    }

    void NavigationPolygon::RemoveOutline(int p_idx)
    {
        m_outlines.erase(m_outlines.begin() + p_idx);
        m_rect_cache_dirty = true;
    }

    void NavigationPolygon::ClearOutlines()
    {
        m_outlines.clear();
        m_rect_cache_dirty = true;
    }

    void NavigationPolygon::AddPolygon(const std::vector<int>& p_polygon)
    {
        Polygon polygon;
        polygon.indices = p_polygon;
        m_polygons.push_back(polygon);
    }

#define TRIANGULATOR_CCW 1
#define TRIANGULATOR_CW -1

    class TriangulatorPoly
    {
    public:
        TriangulatorPoly():
            m_points(nullptr),
            m_hole(false),
            m_numpoints(0)
        {
        
        }

        ~TriangulatorPoly()
        {
            this->Clear();
        }

        void Clear()
        {
            if (m_points) delete[] m_points;
            m_points = nullptr;
            m_hole = false;
            m_numpoints = 0;
        }

        void Init(long numpoints)
        {
            this->Clear();
            this->m_numpoints = numpoints;
            m_points = new Vector2[numpoints];
        }

        Vector2& operator[] (int i)
        {
            return m_points[i];
        }

        Vector2& GetPoint(long i)
        {
            return m_points[i];
        }

        void Triangle(Vector2& p1, Vector2& p2, Vector2& p3)
        {
            this->Init(3);
            m_points[0] = p1;
            m_points[1] = p2;
            m_points[2] = p3;
        }

        TriangulatorPoly& operator =(const TriangulatorPoly& src)
        {
            this->Clear();
            m_hole = src.m_hole;
            m_numpoints = src.m_numpoints;
            m_points = new Vector2[m_numpoints];
            memcpy(m_points, src.m_points, m_numpoints * sizeof(Vector2));
            return *this;
        }

        int GetOrientation()
        {
            long i1, i2;
            float area = 0;
            for (i1 = 0; i1 < m_numpoints; i1++)
            {
                i2 = i1 + 1;
                if (i2 == m_numpoints) i2 = 0;
                area += m_points[i1].x * m_points[i2].y - m_points[i1].y * m_points[i2].x;
            }
            if (area > 0) return TRIANGULATOR_CCW;
            if (area < 0) return TRIANGULATOR_CW;
            return 0;
        }

        void SetOrientation(int orientation)
        {
            int polyorientation = this->GetOrientation();
            if (polyorientation && (polyorientation != orientation))
            {
                this->Invert();
            }
        }

        void Invert()
        {
            long i;
            Vector2* invpoints;

            invpoints = new Vector2[m_numpoints];
            for (i = 0; i < m_numpoints; i++)
            {
                invpoints[i] = m_points[m_numpoints - i - 1];
            }

            delete[] m_points;
            m_points = invpoints;
        }

        void SetHole(bool hole)
        {
            m_hole = hole;
        }

        bool IsHole() const
        {
            return m_hole;
        }

        long GetNumPoints() const
        {
            return m_numpoints;
        }

    private:
        Vector2* m_points;
        bool m_hole;
        long m_numpoints;
    };

    struct PartitionVertex
    {
        bool isActive = false;
        bool isConvex = false;
        bool isEar = false;

        Vector2 p;
        float angle = 0.0f;
        PartitionVertex* previous = nullptr;
        PartitionVertex* next = nullptr;
    };

    class TriangulatorPartition
    {
    public:
        bool IsInside(Vector2& p1, Vector2& p2, Vector2& p3, Vector2& p)
        {
            if (this->IsConvex(p1, p, p2)) return false;
            if (this->IsConvex(p2, p, p3)) return false;
            if (this->IsConvex(p3, p, p1)) return false;
            return true;
        }

        void UpdateVertex(PartitionVertex* v, PartitionVertex* vertices, long numvertices)
        {
            long i;
            PartitionVertex *v1, *v3;
            Vector2 vec1, vec3;

            v1 = v->previous;
            v3 = v->next;

            v->isConvex = this->IsConvex(v1->p, v->p, v3->p);

            vec1 = this->Normalize(v1->p - v->p);
            vec3 = this->Normalize(v3->p - v->p);
            v->angle = vec1.x*vec3.x + vec1.y*vec3.y;

            if (v->isConvex)
            {
                v->isEar = true;
                for (i = 0; i < numvertices; i++)
                {
                    if ((vertices[i].p.x == v->p.x) && (vertices[i].p.y == v->p.y)) continue;
                    if ((vertices[i].p.x == v1->p.x) && (vertices[i].p.y == v1->p.y)) continue;
                    if ((vertices[i].p.x == v3->p.x) && (vertices[i].p.y == v3->p.y)) continue;
                    if (this->IsInside(v1->p, v->p, v3->p, vertices[i].p))
                    {
                        v->isEar = false;
                        break;
                    }
                }
            }
            else
            {
                v->isEar = false;
            }
        }

        int Triangulate_EC(TriangulatorPoly* poly, std::list<TriangulatorPoly>* triangles)
        {
            long numvertices;
            PartitionVertex* vertices = nullptr;
            PartitionVertex* ear = nullptr;
            TriangulatorPoly triangle;
            long i, j;
            bool earfound;

            if (poly->GetNumPoints() < 3) return 0;
            if (poly->GetNumPoints() == 3)
            {
                triangles->push_back(*poly);
                return 1;
            }

            numvertices = poly->GetNumPoints();

            vertices = new PartitionVertex[numvertices];
            for (i = 0; i < numvertices; i++)
            {
                vertices[i].isActive = true;
                vertices[i].p = poly->GetPoint(i);
                if (i == (numvertices - 1)) vertices[i].next = &(vertices[0]);
                else vertices[i].next = &(vertices[i + 1]);
                if (i == 0) vertices[i].previous = &(vertices[numvertices - 1]);
                else vertices[i].previous = &(vertices[i - 1]);
            }
            for (i = 0; i < numvertices; i++)
            {
                this->UpdateVertex(&vertices[i], vertices, numvertices);
            }

            for (i = 0; i < numvertices - 3; i++)
            {
                earfound = false;
                //find the most extruded ear
                for (j = 0; j < numvertices; j++)
                {
                    if (!vertices[j].isActive) continue;
                    if (!vertices[j].isEar) continue;
                    if (!earfound)
                    {
                        earfound = true;
                        ear = &(vertices[j]);
                    }
                    else
                    {
                        if (vertices[j].angle > ear->angle)
                        {
                            ear = &(vertices[j]);
                        }
                    }
                }
                if (!earfound)
                {
                    delete[] vertices;
                    return 0;
                }

                triangle.Triangle(ear->previous->p, ear->p, ear->next->p);
                triangles->push_back(triangle);

                ear->isActive = false;
                ear->previous->next = ear->next;
                ear->next->previous = ear->previous;

                if (i == numvertices - 4) break;

                this->UpdateVertex(ear->previous, vertices, numvertices);
                this->UpdateVertex(ear->next, vertices, numvertices);
            }
            for (i = 0; i < numvertices; i++)
            {
                if (vertices[i].isActive)
                {
                    triangle.Triangle(vertices[i].previous->p, vertices[i].p, vertices[i].next->p);
                    triangles->push_back(triangle);
                    break;
                }
            }

            delete[] vertices;

            return 1;
        }

        bool IsReflex(Vector2& p1, Vector2& p2, Vector2& p3)
        {
            float tmp;
            tmp = (p3.y - p1.y)*(p2.x - p1.x) - (p3.x - p1.x)*(p2.y - p1.y);
            if (tmp < 0) return true;
            else return false;
        }

        int ConvexPartition_HM(TriangulatorPoly* poly, std::list<TriangulatorPoly>* parts)
        {
            std::list<TriangulatorPoly> triangles;
            std::list<TriangulatorPoly>::iterator iter2;
            TriangulatorPoly *poly1 = nullptr, *poly2 = nullptr;
            TriangulatorPoly newpoly;
            Vector2 d1, d2, p1, p2, p3;
            long i11, i12, i21, i22, i13, i23, j, k;
            bool isdiagonal;
            long numreflex;

            //check if the poly is already convex
            numreflex = 0;
            for (i11 = 0; i11 < poly->GetNumPoints(); i11++)
            {
                if (i11 == 0) i12 = poly->GetNumPoints() - 1;
                else i12 = i11 - 1;
                if (i11 == (poly->GetNumPoints() - 1)) i13 = 0;
                else i13 = i11 + 1;
                if (IsReflex(poly->GetPoint(i12), poly->GetPoint(i11), poly->GetPoint(i13)))
                {
                    numreflex = 1;
                    break;
                }
            }
            if (numreflex == 0)
            {
                parts->push_back(*poly);
                return 1;
            }

            if (!Triangulate_EC(poly, &triangles)) return 0;

            for (auto iter1 = triangles.begin(); iter1 != triangles.end(); ++iter1)
            {
                poly1 = &(*iter1);
                for (i11 = 0; i11 < poly1->GetNumPoints(); i11++)
                {
                    d1 = poly1->GetPoint(i11);
                    i12 = (i11 + 1) % (poly1->GetNumPoints());
                    d2 = poly1->GetPoint(i12);

                    isdiagonal = false;
                    for (iter2 = iter1; iter2 != triangles.end(); ++iter2)
                    {
                        if (iter1 == iter2) continue;
                        poly2 = &(*iter2);

                        for (i21 = 0; i21 < poly2->GetNumPoints(); i21++)
                        {
                            if ((d2.x != poly2->GetPoint(i21).x) || (d2.y != poly2->GetPoint(i21).y)) continue;
                            i22 = (i21 + 1) % (poly2->GetNumPoints());
                            if ((d1.x != poly2->GetPoint(i22).x) || (d1.y != poly2->GetPoint(i22).y)) continue;
                            isdiagonal = true;
                            break;
                        }
                        if (isdiagonal) break;
                    }

                    if (!isdiagonal) continue;

                    p2 = poly1->GetPoint(i11);
                    if (i11 == 0) i13 = poly1->GetNumPoints() - 1;
                    else i13 = i11 - 1;
                    p1 = poly1->GetPoint(i13);
                    if (i22 == (poly2->GetNumPoints() - 1)) i23 = 0;
                    else i23 = i22 + 1;
                    p3 = poly2->GetPoint(i23);

                    if (!IsConvex(p1, p2, p3)) continue;

                    p2 = poly1->GetPoint(i12);
                    if (i12 == (poly1->GetNumPoints() - 1)) i13 = 0;
                    else i13 = i12 + 1;
                    p3 = poly1->GetPoint(i13);
                    if (i21 == 0) i23 = poly2->GetNumPoints() - 1;
                    else i23 = i21 - 1;
                    p1 = poly2->GetPoint(i23);

                    if (!IsConvex(p1, p2, p3)) continue;

                    newpoly.Init(poly1->GetNumPoints() + poly2->GetNumPoints() - 2);
                    k = 0;
                    for (j = i12; j != i11; j = (j + 1) % (poly1->GetNumPoints()))
                    {
                        newpoly[k] = poly1->GetPoint(j);
                        k++;
                    }
                    for (j = i22; j != i21; j = (j + 1) % (poly2->GetNumPoints()))
                    {
                        newpoly[k] = poly2->GetPoint(j);
                        k++;
                    }

                    triangles.erase(iter2);
                    *iter1 = newpoly;
                    poly1 = &(*iter1);
                    i11 = -1;

                    continue;
                }
            }

            for (auto& it : triangles)
            {
                parts->push_back(it);
            }

            return 1;
        }

        int ConvexPartition_HM(std::list<TriangulatorPoly>* inpolys, std::list<TriangulatorPoly>* parts)
        {
            std::list<TriangulatorPoly> outpolys;

            if (!RemoveHoles(inpolys, &outpolys)) return 0;
            for (auto& iter : outpolys)
            {
                if (!ConvexPartition_HM(&iter, parts)) return 0;
            }
            return 1;
        }

        bool IsConvex(Vector2& p1, Vector2& p2, Vector2& p3)
        {
            float tmp;
            tmp = (p3.y - p1.y)*(p2.x - p1.x) - (p3.x - p1.x)*(p2.y - p1.y);
            if (tmp > 0) return 1;
            else return 0;
        }

        bool InCone(Vector2& p1, Vector2& p2, Vector2& p3, Vector2& p)
        {
            bool convex;

            convex = IsConvex(p1, p2, p3);

            if (convex)
            {
                if (!IsConvex(p1, p2, p)) return false;
                if (!IsConvex(p2, p3, p)) return false;
                return true;
            }
            else
            {
                if (IsConvex(p1, p2, p)) return true;
                if (IsConvex(p2, p3, p)) return true;
                return false;
            }
        }

        Vector2 Normalize(const Vector2& p)
        {
            Vector2 r;
            float n = sqrt(p.x*p.x + p.y*p.y);
            if (n != 0)
            {
                r = p * (1.0f / n);
            }
            else
            {
                r.x = 0;
                r.y = 0;
            }
            return r;
        }

        int Intersects(Vector2& p11, Vector2& p12, Vector2& p21, Vector2& p22)
        {
            if (Mathf::FloatEqual(p11.x, p21.x) && Mathf::FloatEqual(p11.y, p21.y)) return 0;
            if (Mathf::FloatEqual(p11.x, p22.x) && Mathf::FloatEqual(p11.y, p22.y)) return 0;
            if (Mathf::FloatEqual(p12.x, p21.x) && Mathf::FloatEqual(p12.y, p21.y)) return 0;
            if (Mathf::FloatEqual(p12.x, p22.x) && Mathf::FloatEqual(p12.y, p22.y)) return 0;

            Vector2 v1ort, v2ort, v;
            float dot11, dot12, dot21, dot22;

            v1ort.x = p12.y - p11.y;
            v1ort.y = p11.x - p12.x;

            v2ort.x = p22.y - p21.y;
            v2ort.y = p21.x - p22.x;

            v = p21 - p11;
            dot21 = v.x*v1ort.x + v.y*v1ort.y;
            v = p22 - p11;
            dot22 = v.x*v1ort.x + v.y*v1ort.y;

            v = p11 - p21;
            dot11 = v.x*v2ort.x + v.y*v2ort.y;
            v = p12 - p21;
            dot12 = v.x*v2ort.x + v.y*v2ort.y;

            if (dot11*dot12 > 0) return 0;
            if (dot21*dot22 > 0) return 0;

            return 1;
        }

        int RemoveHoles(std::list<TriangulatorPoly>* inpolys, std::list<TriangulatorPoly>* outpolys)
        {
            std::list<TriangulatorPoly> polys;
            TriangulatorPoly *holeiter = nullptr, *polyiter = nullptr;
            long i, i2, holepointindex, polypointindex;
            Vector2 holepoint, polypoint, bestpolypoint;
            Vector2 linep1, linep2;
            Vector2 v1, v2;
            TriangulatorPoly newpoly;
            bool hasholes;
            bool pointvisible;
            bool pointfound;

            //check for trivial case (no holes)
            hasholes = false;
            for (auto& it : *inpolys)
            {
                if (it.IsHole())
                {
                    hasholes = true;
                    break;
                }
            }
            if (!hasholes)
            {
                for (auto& it : *inpolys)
                {
                    outpolys->push_back(it);
                }
                return 1;
            }

            polys = *inpolys;

            while (1)
            {
                //find the hole point with the largest x
                hasholes = false;
                for (auto& it : polys)
                {
                    if (!it.IsHole()) continue;

                    if (!hasholes)
                    {
                        hasholes = true;
                        holeiter = &it;
                        holepointindex = 0;
                    }

                    for (i = 0; i < it.GetNumPoints(); i++)
                    {
                        if (it.GetPoint(i).x > holeiter->GetPoint(holepointindex).x)
                        {
                            holeiter = &it;
                            holepointindex = i;
                        }
                    }
                }
                if (!hasholes) break;
                holepoint = holeiter->GetPoint(holepointindex);

                pointfound = false;
                for (auto& it : polys)
                {
                    if (it.IsHole()) continue;
                    for (i = 0; i < it.GetNumPoints(); i++)
                    {
                        if (it.GetPoint(i).x <= holepoint.x) continue;
                        if (!InCone(it.GetPoint((i + it.GetNumPoints() - 1) % (it.GetNumPoints())),
                            it.GetPoint(i),
                            it.GetPoint((i + 1) % (it.GetNumPoints())),
                            holepoint))
                            continue;
                        polypoint = it.GetPoint(i);
                        if (pointfound)
                        {
                            v1 = Normalize(polypoint - holepoint);
                            v2 = Normalize(bestpolypoint - holepoint);
                            if (v2.x > v1.x) continue;
                        }
                        pointvisible = true;
                        for (auto& it2 : polys)
                        {
                            if (it2.IsHole()) continue;
                            for (i2 = 0; i2 < it2.GetNumPoints(); i2++)
                            {
                                linep1 = it2.GetPoint(i2);
                                linep2 = it2.GetPoint((i2 + 1) % (it2.GetNumPoints()));
                                if (Intersects(holepoint, polypoint, linep1, linep2))
                                {
                                    pointvisible = false;
                                    break;
                                }
                            }
                            if (!pointvisible) break;
                        }
                        if (pointvisible)
                        {
                            pointfound = true;
                            bestpolypoint = polypoint;
                            polyiter = &it;
                            polypointindex = i;
                        }
                    }
                }

                if (!pointfound) return 0;

                newpoly.Init(holeiter->GetNumPoints() + polyiter->GetNumPoints() + 2);
                i2 = 0;
                for (i = 0; i <= polypointindex; i++)
                {
                    newpoly[i2] = polyiter->GetPoint(i);
                    i2++;
                }
                for (i = 0; i <= holeiter->GetNumPoints(); i++)
                {
                    newpoly[i2] = holeiter->GetPoint((i + holepointindex) % holeiter->GetNumPoints());
                    i2++;
                }
                for (i = polypointindex; i < polyiter->GetNumPoints(); i++)
                {
                    newpoly[i2] = polyiter->GetPoint(i);
                    i2++;
                }

                polys.remove_if([=](const TriangulatorPoly& p) {
                    return holeiter == &p;
                });
                polys.remove_if([=](const TriangulatorPoly& p) {
                    return polyiter == &p;
                });
                polys.push_back(newpoly);
            }

            for (auto& it : polys)
            {
                outpolys->push_back(it);
            }

            return 1;
        }
    };
    
    static bool operator <(const Vector2& a, const Vector2& b) { return Mathf::FloatEqual(a.x, b.x) ? (a.y < b.y) : (a.x < b.x); }

    void NavigationPolygon::MakePolygonsFromOutlines()
    {
        std::list<TriangulatorPoly> in_poly, out_poly;

        Vector2 outside_point(-1e10f, -1e10f);

        for (int i = 0; i < (int) m_outlines.size(); i++)
        {
            const auto& ol = m_outlines[i];
            int olsize = (int) ol.size();
            if (olsize < 3)
                continue;
            const auto& r = ol;
            for (int j = 0; j < olsize; j++)
            {
                outside_point.x = std::max(r[j].x, outside_point.x);
                outside_point.y = std::max(r[j].y, outside_point.y);
            }
        }

        outside_point += Vector2(0.7239784f, 0.819238f); //avoid precision issues

        for (int i = 0; i < (int) m_outlines.size(); i++)
        {
            const auto& ol = m_outlines[i];
            int olsize = (int) ol.size();
            if (olsize < 3)
                continue;
            const auto& r = ol;

            int interscount = 0;
            //test if this is an outer outline
            for (int k = 0; k < (int) m_outlines.size(); k++)
            {
                if (i == k)
                    continue; //no self intersect

                const auto& ol2 = m_outlines[k];
                int olsize2 = (int) ol2.size();
                if (olsize2 < 3)
                    continue;
                const auto& r2 = ol2;

                for (int l = 0; l < (int) olsize2; l++)
                {
                    if (segment_intersects_segment_2d(r[0], outside_point, r2[l], r2[(l + 1) % olsize2], nullptr))
                    {
                        interscount++;
                    }
                }
            }

            bool outer = (interscount % 2) == 0;

            TriangulatorPoly tp;
            tp.Init(olsize);
            for (int j = 0; j < olsize; j++)
            {
                tp[j] = r[j];
            }

            if (outer)
                tp.SetOrientation(TRIANGULATOR_CCW);
            else {
                tp.SetOrientation(TRIANGULATOR_CW);
                tp.SetHole(true);
            }

            in_poly.push_back(tp);
        }

        TriangulatorPartition tpart;
        if (tpart.ConvexPartition_HM(&in_poly, &out_poly) == 0)
        {
            //failed!
            Log("NavigationPolygon: Convex partition failed!");
            return;
        }

        m_polygons.clear();
        m_vertices.resize(0);

        std::map<Vector2, int> points;
        for (auto& I : out_poly)
        {
            TriangulatorPoly& tp = I;
            struct Polygon p;

            for (int i = 0; i < (int) tp.GetNumPoints(); i++)
            {
                auto E = points.find(tp[i]);
                if (E == points.end())
                {
                    points[tp[i]] = (int) m_vertices.size();
                    E = points.find(tp[i]);
                    m_vertices.push_back(tp[i]);
                }
                p.indices.push_back(E->second);
            }

            m_polygons.push_back(p);
        }
    }

    NavigationPolygonInstance::NavigationPolygonInstance():
        m_enabled(true),
        m_nav_id(-1)
    {
        m_navigation = std::shared_ptr<Navigation2D>(new Navigation2D());
    }

    void NavigationPolygonInstance::SetNavigationPolygon(const std::shared_ptr<NavigationPolygon>& p_navpoly)
    {
        if (p_navpoly == m_navpoly)
        {
            return;
        }

        if (m_navigation && m_nav_id != -1)
        {
            m_navigation->NavpolyRemove(m_nav_id);
            m_nav_id = -1;
        }

        m_navpoly = p_navpoly;

        if (m_navigation && m_navpoly && m_enabled)
        {
            m_nav_id = m_navigation->NavpolyAdd(m_navpoly, Transform2D(), this);
        }
    }

    void NavigationPolygonInstance::SetEnabled(bool p_enabled)
    {
        if (m_enabled == p_enabled)
            return;
        m_enabled = p_enabled;

        if (!m_enabled)
        {
            if (m_nav_id != -1)
            {
                m_navigation->NavpolyRemove(m_nav_id);
                m_nav_id = -1;
            }
        }
        else
        {
            if (m_navigation)
            {
                if (m_navpoly)
                {
                    m_nav_id = m_navigation->NavpolyAdd(m_navpoly, Transform2D(), this);
                }
            }
        }
    }
}
