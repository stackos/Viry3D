/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "Display.h"
#include "string/String.h"
#include "RenderState.h"

namespace Viry3D
{
    class Shader
    {
    public:
        Shader(
            const String& vs_source,
            const Vector<String>& vs_includes,
            const String& fs_source,
            const Vector<String>& fs_includes,
            const RenderState& render_state);
        ~Shader();
        const RenderState& GetRenderState() const { return m_render_state; }

    private:
        RenderState m_render_state;
        VkShaderModule m_vs_module;
        VkShaderModule m_fs_module;
        Vector<UniformSet> m_uniform_sets;
    };
}
