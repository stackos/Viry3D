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
    class NavigationPolygon
    {
    public:
        std::vector<Vector2>& GetVertices() { return m_vertices; }
        int GetPolygonCount() const { return (int) m_polygons.size(); }
        std::vector<int>& GetPolygon(int index) { return m_polygons[index].indices; }

    private:
        struct Polygon
        {
            std::vector<int> indices;
        };

        std::vector<Vector2> m_vertices;
        std::vector<Polygon> m_polygons;
    };
}
