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

#include "Vector3.h"
#include "container/Vector.h"

namespace Viry3D
{
	class Curve
	{
	public:
        Curve();
        virtual ~Curve();
        virtual Vector3 GetPoint(float t) const = 0;
        Vector3 GetPointAt(float u);
        Vector<Vector3> GetPoints(int divisions = 5) const;
        Vector<Vector3> GetSpacedPoints(int divisions = 5);
        float GetLength();
        const Vector<float>& GetLengths(int divisions = -1);
        void UpdateArcLengths();
        float GetUtoTmapping(float u, float distance = -1);
        Vector3 GetTangent(float t) const;
        Vector3 GetTangentAt(float u);
        void ComputeFrenetFrames(int segments, bool closed, Vector<Vector3>& tangents, Vector<Vector3>& normals, Vector<Vector3>& binormals);
        
	private:
        int m_arc_length_divisions = 200;
        Vector<float> m_cache_arc_lengths;
        bool m_needs_update = true;
	};
}
