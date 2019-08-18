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

#include "Curve.h"
#include "Mathf.h"
#include "Matrix4x4.h"

namespace Viry3D
{
    Curve::Curve()
    {
        
    }
    
    Curve::~Curve()
    {
        
    }
    
    Vector3 Curve::GetPointAt(float u)
    {
        float t = this->GetUtoTmapping(u);
        return this->GetPoint(t);
    }
    
    Vector<Vector3> Curve::GetPoints(int divisions) const
    {
        Vector<Vector3> points;
        
        for (int i = 0; i <= divisions; ++i)
        {
            points.Add(this->GetPoint(i / (float) divisions));
        }
        
        return points;
    }
    
    Vector<Vector3> Curve::GetSpacedPoints(int divisions)
    {
        Vector<Vector3> points;
        
        for (int i = 0; i <= divisions; ++i)
        {
            points.Add(this->GetPointAt(i / (float) divisions));
        }
        
        return points;
    }
    
    float Curve::GetLength()
    {
        const auto& lengths = this->GetLengths();
        return lengths[lengths.Size() - 1];
    }
    
    const Vector<float>& Curve::GetLengths(int divisions)
    {
        if (divisions < 0)
        {
            divisions = m_arc_length_divisions;
        }
        
        if (m_cache_arc_lengths.Size() == divisions + 1 && !m_needs_update)
        {
            return m_cache_arc_lengths;
        }
        
        m_needs_update = false;
        
        Vector3 current = this->GetPoint(0);
        Vector3 last = current;
        float sum = 0;
        
        m_cache_arc_lengths.Clear();
        m_cache_arc_lengths.Add(0);
        
        for (int i = 1; i <= divisions; ++i)
        {
            current = this->GetPoint(i / (float) divisions);
            sum += Vector3::Distance(current, last);
            m_cache_arc_lengths.Add(sum);
            last = current;
        }
        
        return m_cache_arc_lengths;
    }
    
    void Curve::UpdateArcLengths()
    {
        m_needs_update = true;
        this->GetLengths();
    }
    
    float Curve::GetUtoTmapping(float u, float distance)
    {
        const auto& lengths = this->GetLengths();
        
        int il = lengths.Size();
        float target_arc_length = 0;
        
        if (distance >= 0)
        {
            target_arc_length = distance;
        }
        else
        {
            target_arc_length = u * lengths[il - 1];
        }
        
        int i = 0;
        int low = 0;
        int high = il - 1;
        
        while (low <= high)
        {
            i = (int) Mathf::Floor(low + (high - low) / 2.0f);
            float comparison = lengths[i] - target_arc_length;
            if (comparison < 0)
            {
                low = i + 1;
            }
            else if (comparison > 0)
            {
                high = i - 1;
            }
            else
            {
                high = i;
                break;
            }
        }
        i = high;
        
        if (Mathf::FloatEqual(lengths[i], target_arc_length))
        {
            return i / (float) (il - 1);
        }
        
        float length_before = lengths[i];
        float length_after = lengths[i + 1];
        float segment_length = length_after - length_before;
        float segment_fraction = (target_arc_length - length_before) / segment_length;
        float t = (i + segment_fraction) / (float) (il - 1);
        
        return t;
    }
    
    Vector3 Curve::GetTangent(float t) const
    {
        float delta = 0.0001f;
        float t1 = t - delta;
        float t2 = t + delta;
        
        if (t1 < 0) t1 = 0;
        if (t2 > 1) t2 = 1;
        
        Vector3 pt1 = this->GetPoint(t1);
        Vector3 pt2 = this->GetPoint(t2);
        Vector3 vec = pt2 - pt1;
        
        return vec.Normalized();
    }
    
    Vector3 Curve::GetTangentAt(float u)
    {
        float t = this->GetUtoTmapping(u);
        return this->GetTangent(t);
    }
    
    void Curve::ComputeFrenetFrames(int segments, bool closed, Vector<Vector3>& tangents, Vector<Vector3>& normals, Vector<Vector3>& binormals)
    {
        tangents.Resize(segments + 1);
        normals.Resize(segments + 1);
        binormals.Resize(segments + 1);
        
        for (int i = 0; i <= segments; ++i)
        {
            float u = i / (float) segments;
            
            tangents[i] = this->GetTangentAt(u);
            tangents[i].Normalize();
        }
        
        float min = Mathf::MaxFloatValue;
        float tx = Mathf::Abs(tangents[0].x);
        float ty = Mathf::Abs(tangents[0].y);
        float tz = Mathf::Abs(tangents[0].z);
        Vector3 normal;
        
        if (tx <= min)
        {
            min = tx;
            normal = Vector3(1, 0, 0);
        }
        
        if (ty <= min)
        {
            min = ty;
            normal = Vector3(0, 1, 0);
        }
        
        if (tz <= min)
        {
            normal = Vector3(0, 0, 1);
        }
        
        Vector3 vec = (tangents[0] * normal).Normalized();
        normals[0] = tangents[0] * vec;
        binormals[0] = tangents[0] * normals[0];

        for (int i = 1; i <= segments; ++i)
        {
            normals[i] = normals[i - 1];
            binormals[i] = binormals[i - 1];
            
            vec = tangents[i - 1] * tangents[i];
            
            if (vec.SqrMagnitude() > Mathf::Epsilon)
            {
                vec.Normalize();
                
                float theta = acos(Mathf::Clamp(tangents[i - 1].Dot(tangents[i]), -1.0f, 1.0f));
                
                normals[i] = Quaternion::AngleAxis(theta * Mathf::Rad2Deg, vec) * normals[i];
            }
            
            binormals[i] = tangents[i] * normals[i];
        }
        
        if (closed)
        {
            float theta = acos(Mathf::Clamp(normals[0].Dot(normals[segments]), -1.0f, 1.0f));
            theta /= segments;
            
            if (tangents[0].Dot(normals[0] * normals[segments]) > 0)
            {
                theta = - theta;
            }
            
            for (int i = 1; i <= segments; ++i)
            {
                normals[i] = Quaternion::AngleAxis(theta * i * Mathf::Rad2Deg, tangents[i]) * normals[i];
                binormals[i] = tangents[i] * normals[i];
            }
        }
    }
}
