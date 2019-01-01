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

#include "VertexAttribute.h"

namespace Viry3D
{
    const char* VERTEX_ATTR_NAMES[(int) VertexAttributeType::Count] =
    {
        "a_pos",
        "a_color",
        "a_uv",
        "a_uv2",
        "a_normal",
        "a_tangent",
        "a_bone_weights",
        "a_bone_indices"
    };

    const int VERTEX_ATTR_SIZES[(int) VertexAttributeType::Count] = {
        12, 16, 8, 8, 12, 16, 16, 16
    };

    const int VERTEX_ATTR_OFFSETS[(int) VertexAttributeType::Count] =
    {
        0, 12, 28, 36, 44, 56, 72, 88
    };
}
