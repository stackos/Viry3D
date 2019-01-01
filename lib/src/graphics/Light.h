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

#include "Node.h"
#include "Color.h"

namespace Viry3D
{
    enum class LightType
    {
        Directional,
        Point,
        Spotlight,
    };

    class Light : public Node
    {
    public:
        static const Color& GetAmbientColor() { return m_ambient_color; }
        static void SetAmbientColor(const Color& color);
        Light(LightType type);
        virtual ~Light();
        LightType GetType() const { return m_type; }
        const Color& GetColor() const { return m_color; }
        void SetColor(const Color& color);
        const float GetIntensity() const { return m_intensity; }
        void SetIntensity(float intensity);

    private:
        static Color m_ambient_color;
        LightType m_type;
        Color m_color;
        float m_intensity;
    };
}
