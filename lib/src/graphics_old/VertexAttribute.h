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

#include "Color.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

namespace Viry3D
{
	enum class VertexAttributeType
	{
		None = -1,

		Vertex = 0,
		Color,
		Texcoord,
		Texcoord2,
		Normal,
		Tangent,
		BlendWeight,
		BlendIndices,

		Count
	};

    enum class InstanceVertexAttributeLocation
    {
        TransformMatrixRow0 = (int) VertexAttributeType::Count,
        TransformMatrixRow1,
        TransformMatrixRow2,
        TransformMatrixRow3,
    };

	struct Vertex
	{
		Vector3 vertex;
		Color color;
		Vector2 uv;
		Vector2 uv2;
		Vector3 normal;
		Vector4 tangent;
		Vector4 bone_weight;
		Vector4 bone_indices;
	};

	extern const char* VERTEX_ATTR_NAMES[(int) VertexAttributeType::Count];
	extern const int VERTEX_ATTR_SIZES[(int) VertexAttributeType::Count];
	extern const int VERTEX_ATTR_OFFSETS[(int) VertexAttributeType::Count];

    struct VertexAttribute
    {
        String name;
        int location;
        int vector_size;
    };
}
