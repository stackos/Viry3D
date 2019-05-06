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

#include "Display.h"
#include "RenderState.h"
#include "string/String.h"
#include "container/List.h"
#include "container/Map.h"

namespace Viry3D
{
#if VR_VULKAN
    struct Pipeline
    {
        VkPipeline pipeline;
        bool instancing;
    };
#endif

    class Shader
    {
    public:
		static Ref<Shader> Find(const String& name);
		static void AddCache(const String& name, const Ref<Shader>& shader);
        static void RemoveCache(const String& name);
		static void Done();
        Shader(
            const String& vs_predefine,
            const Vector<String>& vs_includes,
            const String& vs_source,
            const String& fs_predefine,
            const Vector<String>& fs_includes,
            const String& fs_source,
            const RenderState& render_state);
        Shader(const String& cs_source);
        ~Shader();
        const RenderState& GetRenderState() const { return m_render_state; }
        bool IsComputeShader() const { return m_compute_shader; }
#if VR_VULKAN
        static void OnRenderPassDestroy(VkRenderPass render_pass);
        VkPipeline GetPipeline(VkRenderPass render_pass, bool color_attachment, bool depth_attachment, int extra_color_attachment_count, int sample_count, bool instancing, int instance_stride);
        VkPipeline GetComputePipeline();
        void CreateDescriptorSets(Vector<VkDescriptorSet>& descriptor_sets, Vector<UniformSet>& uniform_sets);
        VkPipelineLayout GetPipelineLayout() const { return m_pipeline_layout; }
#elif VR_GLES
        bool Use() const;
        void EnableVertexAttribs() const;
        void DisableVertexAttribs() const;
        void SetUniform1f(const String& name, float value) const;
        void SetUniform4f(const String& name, int count, const float* value) const;
        void SetUniform1i(const String& name, int value) const;
        void SetUniformMatrix(const String& name, int count, const float* value) const;
        void ApplyRenderState();
#endif

    private:
#if VR_GLES
        struct Attribute
        {
            String name;
            GLenum type;
            int size;
            int loc;
            int vector_size;
        };

        struct Uniform
        {
            String name;
            GLenum type;
            int size;
            int loc;
        };

        void CreateProgram(
            const String& vs_predefine,
            const Vector<String>& vs_includes,
            const String& vs_source,
            const String& fs_predefine,
            const Vector<String>& fs_includes,
            const String& fs_source);
#endif

    private:
        static List<Shader*> m_shaders;
		static Map<String, Ref<Shader>> m_shader_cache;
#if VR_VULKAN
        VkShaderModule m_cs_module;
        VkShaderModule m_vs_module;
        VkShaderModule m_fs_module;
        VkPipelineCache m_pipeline_cache;
        Vector<VkDescriptorSetLayout> m_descriptor_layouts;
        VkPipelineLayout m_pipeline_layout;
        VkDescriptorPool m_descriptor_pool;
        Map<VkRenderPass, Vector<Pipeline>> m_pipelines;
        VkPipeline m_compute_pipeline;
        Vector<VertexAttribute> m_attributes;
        Vector<UniformSet> m_uniform_sets;
#elif VR_GLES
        GLuint m_program;
        Vector<Attribute> m_attributes;
        Vector<Uniform> m_uniforms;
#endif
        RenderState m_render_state;
        bool m_compute_shader;
    };
}
