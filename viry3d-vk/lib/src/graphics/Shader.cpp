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

#include "Shader.h"
#include "Application.h"
#include "io/File.h"

namespace Viry3D
{
    Shader::Shader(
        const String& vs_source,
        const Vector<String>& vs_includes,
        const String& fs_source,
        const Vector<String>& fs_includes,
        const RenderState& render_state):
        m_render_state(render_state),
        m_vs_module(nullptr),
        m_fs_module(nullptr)
    {
        Display::GetDisplay()->CreateShaderModule(
            vs_source,
            vs_includes,
            fs_source,
            fs_includes,
            &m_vs_module,
            &m_fs_module,
            m_uniform_sets);
    }

    Shader::~Shader()
    {
        VkDevice device = Display::GetDisplay()->GetDevice();

        vkDestroyShaderModule(device, m_vs_module, nullptr);
        vkDestroyShaderModule(device, m_fs_module, nullptr);
    }
}
