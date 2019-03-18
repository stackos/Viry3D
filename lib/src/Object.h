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

#include "string/String.h"

namespace Viry3D
{
    class Object
    {
    public:
        Object() { static uint32_t s_id = 0; m_id = ++s_id; }
        virtual ~Object() { }
        const String& GetName() const { return m_name; }
        void SetName(const String& name) { m_name = name; }
        uint32_t GetId() const { return m_id; }

    private:
        String m_name;
        uint32_t m_id;
    };
}
