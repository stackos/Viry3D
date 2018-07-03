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
#include "Camera.h"

namespace Viry3D
{
    List<Shader*> Shader::m_shaders;

    Shader::Shader(
        const String& vs_source,
        const Vector<String>& vs_includes,
        const String& fs_source,
        const Vector<String>& fs_includes,
        const RenderState& render_state):
		m_vs_source(vs_source),
		m_fs_source(fs_source),
        m_render_state(render_state),
        m_vs_module(nullptr),
        m_fs_module(nullptr),
        m_pipeline_cache(nullptr),
        m_pipeline_layout(nullptr),
        m_descriptor_pool(nullptr)
    {
        m_shaders.AddLast(this);

        Display::Instance()->CreateShaderModule(
            vs_source,
            vs_includes,
            fs_source,
            fs_includes,
            &m_vs_module,
            &m_fs_module,
            m_uniform_sets);
        Display::Instance()->CreatePipelineCache(&m_pipeline_cache);
        Display::Instance()->CreatePipelineLayout(m_uniform_sets, m_descriptor_layouts, &m_pipeline_layout);
        Display::Instance()->CreateDescriptorSetPool(m_uniform_sets, &m_descriptor_pool);
    }

    Shader::~Shader()
    {
        VkDevice device = Display::Instance()->GetDevice();

        for (auto i : m_pipelines)
        {
            vkDestroyPipeline(device, i.second, nullptr);
        }
        m_pipelines.Clear();
        vkDestroyDescriptorPool(device, m_descriptor_pool, nullptr);
        vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
        for (int i = 0; i < m_descriptor_layouts.Size(); ++i)
        {
            vkDestroyDescriptorSetLayout(device, m_descriptor_layouts[i], nullptr);
        }
        m_descriptor_layouts.Clear();
        vkDestroyPipelineCache(device, m_pipeline_cache, nullptr);
        vkDestroyShaderModule(device, m_vs_module, nullptr);
        vkDestroyShaderModule(device, m_fs_module, nullptr);

        m_shaders.Remove(this);
    }

    VkPipeline Shader::GetPipeline(VkRenderPass render_pass, bool color_attachment, bool depth_attachment)
    {
        VkPipeline* pipeline_ptr;
        if (m_pipelines.TryGet(render_pass, &pipeline_ptr))
        {
            return *pipeline_ptr;
        }
        else
        {
            VkPipeline pipeline;
            Display::Instance()->CreatePipeline(
                render_pass,
                m_vs_module,
                m_fs_module,
                m_render_state,
                m_pipeline_layout,
                m_pipeline_cache,
                &pipeline,
                color_attachment,
                depth_attachment);
            m_pipelines.Add(render_pass, pipeline);

            return pipeline;
        }
    }

    void Shader::OnRenderPassDestroy(VkRenderPass render_pass)
    {
        VkDevice device = Display::Instance()->GetDevice();

        for (auto i : m_shaders)
        {
            VkPipeline* pipeline = nullptr;
            if (i->m_pipelines.TryGet(render_pass, &pipeline))
            {
                vkDestroyPipeline(device, *pipeline, nullptr);
                i->m_pipelines.Remove(render_pass);
            }
        }
    }

    void Shader::CreateDescriptorSets(Vector<VkDescriptorSet>& descriptor_sets, Vector<UniformSet>& uniform_sets)
    {
        VkDevice device = Display::Instance()->GetDevice();

        Display::Instance()->CreateDescriptorSets(
            m_uniform_sets,
            m_descriptor_pool,
            m_descriptor_layouts,
            descriptor_sets,
            uniform_sets);
    }
}
