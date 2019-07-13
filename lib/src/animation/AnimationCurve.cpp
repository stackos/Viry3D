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

#include "AnimationCurve.h"
#include "math/Mathf.h"

namespace Viry3D
{
    float AnimationCurve::Evaluate(float time, const Key& k0, const Key& k1)
    {
        float dt = k1.time - k0.time;
        if (fabs(dt) < Mathf::Epsilon)
        {
            return k0.value;
        }

        float t = (time - k0.time) / dt;
        float t2 = t * t;
        float t3 = t2 * t;
        float _t = 1 - t;
        float _t2 = _t * _t;
        float _t3 = _t2 * _t;

        float c = 1 / 3.0f;
        float c0 = dt * c * k0.out_tangent + k0.value;
        float c1 = -dt * c * k1.in_tangent + k1.value;
        float value = k0.value * _t3 + 3 * c0 * t * _t2 + 3 * c1 * t2 * _t + k1.value * t3;

        return value;
    }

    AnimationCurve AnimationCurve::Linear(float time_start, float value_start, float time_end, float value_end)
    {
        AnimationCurve curve;
        
        float tangent = (value_end - value_start) / (time_end - time_start);
        curve.AddKey(time_start, value_start, tangent, tangent);
        curve.AddKey(time_end, value_end, tangent, tangent);
        
        return curve;
    }
    
    void AnimationCurve::AddKey(float time, float value, float in_tangent, float out_tangent)
    {
        m_keys.Add(Key({ time, value, in_tangent, out_tangent }));
    }

    float AnimationCurve::Evaluate(float time) const
    {
        if (m_keys.Empty())
        {
            return 0;
        }

        const auto& back = m_keys[m_keys.Size() - 1];
        if (time >= back.time)
        {
            return back.value;
        }

        for (int i = 0; i < m_keys.Size(); ++i)
        {
            const auto& key = m_keys[i];

            if (time < key.time)
            {
                if (i == 0)
                {
                    return key.value;
                }
                else
                {
                    return Evaluate(time, m_keys[i - 1], key);
                }
            }
        }

        return 0;
    }
}
