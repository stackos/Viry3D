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
#include <vector>

namespace Viry3D
{
    struct Rect2
    {
        Rect2():
            pos(0, 0),
            size(0, 0)
        {
        }

        Rect2(const Vector2& pos, const Vector2& size):
            pos(pos),
            size(size)
        {
        }

        inline void ExpandTo(const Vector2 &p_vector)
        {
            Vector2 begin = pos;
            Vector2 end = pos + size;

            if (p_vector.x < begin.x)
                begin.x = p_vector.x;
            if (p_vector.y < begin.y)
                begin.y = p_vector.y;

            if (p_vector.x > end.x)
                end.x = p_vector.x;
            if (p_vector.y > end.y)
                end.y = p_vector.y;

            pos = begin;
            size = end - begin;
        }

        Vector2 pos;
        Vector2 size;
    };

    class NavigationPolygon
    {
    public:
        NavigationPolygon();
        std::vector<Vector2>& GetVertices() { return m_vertices; }
        void SetVertices(const std::vector<Vector2>& p_vertices) { m_vertices = p_vertices; m_rect_cache_dirty = true; }
        int GetPolygonCount() const { return (int) m_polygons.size(); }
        std::vector<int>& GetPolygon(int index) { return m_polygons[index].indices; }
        void AddPolygon(const std::vector<int>& p_polygon);
        void ClearPolygons() { m_polygons.clear(); }
        void MakePolygonsFromOutlines();
        void AddOutline(const std::vector<Vector2>& p_outline);
        void AddOutlineAtIndex(const std::vector<Vector2>& p_outline, int p_index);
        void SetOutline(int p_idx, const std::vector<Vector2>& p_outline);
        const std::vector<Vector2>& GetOutline(int p_idx) const { return m_outlines[p_idx]; }
        void RemoveOutline(int p_idx);
        int GetOutlineCount() const { return (int) m_polygons.size(); }
        void ClearOutlines();

    protected:
        struct Polygon
        {
            std::vector<int> indices;
        };

        void SetPolygons(const std::vector<Polygon>& p_array) { m_polygons = p_array; }
        const std::vector<Polygon>& GetPolygons() const { return m_polygons; }
        void SetOutlines(const std::vector<std::vector<Vector2>>& p_array) { m_outlines = p_array; m_rect_cache_dirty = true; }
        const std::vector<std::vector<Vector2>>& GetOutlines() const { return m_outlines; }

    private:
        std::vector<Vector2> m_vertices;
        std::vector<Polygon> m_polygons;
        std::vector<std::vector<Vector2>> m_outlines;
        mutable Rect2 m_item_rect;
        mutable bool m_rect_cache_dirty;
    };

    class Navigation2D;

    class NavigationPolygonInstance
    {
    public:
        NavigationPolygonInstance();
        void SetNavigationPolygon(const std::shared_ptr<NavigationPolygon>& p_navpoly);
        void SetEnabled(bool p_enabled);
        std::shared_ptr<Navigation2D>& GetNavigation2D() { return m_navigation; }

    private:
        bool m_enabled;
        int m_nav_id;
        std::shared_ptr<Navigation2D> m_navigation;
        std::shared_ptr<NavigationPolygon> m_navpoly;
    };

#define CMP_EPSILON 0.00001f

    bool is_point_in_triangle(const Vector2& s, const Vector2& a, const Vector2& b, const Vector2& c);
}
