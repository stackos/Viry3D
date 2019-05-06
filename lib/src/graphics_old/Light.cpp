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

#include "Light.h"

namespace Viry3D
{
    Color Light::m_ambient_color;

    void Light::SetAmbientColor(const Color& color)
    {
        m_ambient_color = color;
    }

    Light::Light(LightType type):
        m_type(type),
        m_color(1, 1, 1, 1),
        m_intensity(1.0f)
    {
    
    }

    Light::~Light()
    {
        
    }

    void Light::SetColor(const Color& color)
    {
        m_color = color;
    }

    void Light::SetIntensity(float intensity)
    {
        m_intensity = intensity;
    }
}
