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

#include "container/Vector.h"

namespace Viry3D
{
    class AnimationCurve
    {
    private:
        struct Key
        {
            float time;
            float value;
            float in_tangent;
            float out_tangent;
        };

    public:
        void AddKey(float time, float value, float in_tangent, float out_tangent);
        float Evaluate(float time) const;

    private:
        static float Evaluate(float time, const Key& k0, const Key& k1);
        
    private:
        Vector<Key> m_keys;
    };
}
