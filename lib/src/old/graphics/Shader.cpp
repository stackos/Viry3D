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

#include "Shader.h"
#include "Camera.h"
#include "Application.h"
#include "VertexAttribute.h"
#include "io/File.h"
#include "Debug.h"

namespace Viry3D
{
    List<Shader*> Shader::m_shaders;
	Map<String, Ref<Shader>> Shader::m_shader_cache;

	Ref<Shader> Shader::Find(const String& name)
	{
		Ref<Shader> shader;

		Ref<Shader>* find;
		if (m_shader_cache.TryGet(name, &find))
		{
			shader = *find;
		}

		return shader;
	}

	void Shader::AddCache(const String& name, const Ref<Shader>& shader)
	{
		m_shader_cache.Add(name, shader);
	}

    void Shader::RemoveCache(const String& name)
    {
        m_shader_cache.Remove(name);
    }

	void Shader::Done()
	{
		m_shader_cache.Clear();
	}

#if VR_VULKAN
	void Shader::OnRenderPassDestroy(VkRenderPass render_pass)
	{
		VkDevice device = Display::Instance()->GetDevice();

		for (auto i : m_shaders)
		{
            Vector<Pipeline>* pipelines_ptr = nullptr;
			if (i->m_pipelines.TryGet(render_pass, &pipelines_ptr))
			{
                for (auto& j : *pipelines_ptr)
                {
                    vkDestroyPipeline(device, j.pipeline, nullptr);
                }
				i->m_pipelines.Remove(render_pass);
			}
		}
	}
#endif

    Shader::Shader(
        const String& vs_predefine,
        const Vector<String>& vs_includes,
        const String& vs_source,
        const String& fs_predefine,
        const Vector<String>& fs_includes,
        const String& fs_source,
        const RenderState& render_state):
#if VR_VULKAN
        m_cs_module(VK_NULL_HANDLE),
        m_vs_module(VK_NULL_HANDLE),
        m_fs_module(VK_NULL_HANDLE),
        m_pipeline_cache(VK_NULL_HANDLE),
        m_pipeline_layout(VK_NULL_HANDLE),
        m_descriptor_pool(VK_NULL_HANDLE),
        m_compute_pipeline(VK_NULL_HANDLE),
#elif VR_GLES
        m_program(0),
#endif
        m_render_state(render_state),
        m_compute_shader(false)
    {
        m_shaders.AddLast(this);

#if VR_VULKAN
        Display::Instance()->CreateShaderModule(
            vs_predefine,
            vs_includes,
            vs_source,
            fs_predefine,
            fs_includes,
            fs_source,
            &m_vs_module,
            &m_fs_module,
            m_attributes,
            m_uniform_sets);
        Display::Instance()->CreatePipelineCache(&m_pipeline_cache);
        Display::Instance()->CreatePipelineLayout(m_uniform_sets, m_descriptor_layouts, &m_pipeline_layout);
        if (m_uniform_sets.Size() > 0)
        {
            Display::Instance()->CreateDescriptorSetPool(m_uniform_sets, &m_descriptor_pool);
        }
#elif VR_GLES
        this->CreateProgram(
            vs_predefine,
            vs_includes,
            vs_source,
            fs_predefine,
            fs_includes,
            fs_source);
#endif
    }

    Shader::Shader(const String& cs_source):
#if VR_VULKAN
        m_cs_module(VK_NULL_HANDLE),
        m_vs_module(VK_NULL_HANDLE),
        m_fs_module(VK_NULL_HANDLE),
        m_pipeline_cache(VK_NULL_HANDLE),
        m_pipeline_layout(VK_NULL_HANDLE),
        m_descriptor_pool(VK_NULL_HANDLE),
        m_compute_pipeline(VK_NULL_HANDLE),
#elif VR_GLES
        m_program(0),
#endif
        m_compute_shader(true)
    {
        m_shaders.AddLast(this);

#if VR_VULKAN
        Display::Instance()->CreateComputeShaderModule(
            cs_source,
            &m_cs_module,
            m_uniform_sets);
        Display::Instance()->CreatePipelineCache(&m_pipeline_cache);
        Display::Instance()->CreatePipelineLayout(m_uniform_sets, m_descriptor_layouts, &m_pipeline_layout);
        if (m_uniform_sets.Size() > 0)
        {
            Display::Instance()->CreateDescriptorSetPool(m_uniform_sets, &m_descriptor_pool);
        }
#endif
    }

    Shader::~Shader()
    {
#if VR_VULKAN
        VkDevice device = Display::Instance()->GetDevice();

        for (auto i : m_pipelines)
        {
            for (auto& j : i.second)
            {
                vkDestroyPipeline(device, j.pipeline, nullptr);
            }
        }
        m_pipelines.Clear();
        if (m_compute_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, m_compute_pipeline, nullptr);
            m_compute_pipeline = VK_NULL_HANDLE;
        }
        if (m_descriptor_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, m_descriptor_pool, nullptr);
            m_descriptor_pool = VK_NULL_HANDLE;
        }
        if (m_pipeline_layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
            m_pipeline_layout = VK_NULL_HANDLE;
        }
        for (int i = 0; i < m_descriptor_layouts.Size(); ++i)
        {
            vkDestroyDescriptorSetLayout(device, m_descriptor_layouts[i], nullptr);
        }
        m_descriptor_layouts.Clear();
        if (m_pipeline_cache != VK_NULL_HANDLE)
        {
            vkDestroyPipelineCache(device, m_pipeline_cache, nullptr);
            m_pipeline_cache = VK_NULL_HANDLE;
        }
        if (m_fs_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, m_fs_module, nullptr);
            m_fs_module = VK_NULL_HANDLE;
        }
        if (m_vs_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, m_vs_module, nullptr);
            m_vs_module = VK_NULL_HANDLE;
        }
        if (m_cs_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, m_cs_module, nullptr);
            m_cs_module = VK_NULL_HANDLE;
        }
#elif VR_GLES
        if (m_program)
        {
            glDeleteProgram(m_program);
        }
#endif

        m_shaders.Remove(this);
    }

#if VR_VULKAN
    VkPipeline Shader::GetPipeline(VkRenderPass render_pass, bool color_attachment, bool depth_attachment, int extra_color_attachment_count, int sample_count, bool instancing, int instance_stride)
    {
        Vector<Pipeline>* pipelines_ptr = nullptr;
        if (m_pipelines.TryGet(render_pass, &pipelines_ptr))
        {
            for (int i = 0; i < pipelines_ptr->Size(); ++i)
            {
                if ((*pipelines_ptr)[i].instancing == instancing)
                {
                    return (*pipelines_ptr)[i].pipeline;
                }
            }
        }

        Pipeline p;
        p.instancing = instancing;

        Display::Instance()->CreatePipeline(
            render_pass,
            m_attributes,
            m_vs_module,
            m_fs_module,
            m_render_state,
            m_pipeline_layout,
            m_pipeline_cache,
            &p.pipeline,
            color_attachment,
            depth_attachment,
            extra_color_attachment_count,
            sample_count,
            instancing,
            instance_stride);

        if (pipelines_ptr)
        {
            pipelines_ptr->Add(p);
        }
        else
        {
            Vector<Pipeline> pipelines;
            pipelines.Add(p);

            m_pipelines.Add(render_pass, pipelines);
        }

        return p.pipeline;
    }

    VkPipeline Shader::GetComputePipeline()
    {
        if (m_compute_pipeline == VK_NULL_HANDLE)
        {
            Display::Instance()->CreateComputePipeline(
                m_cs_module,
                m_pipeline_layout,
                m_pipeline_cache,
                &m_compute_pipeline);
        }

        return m_compute_pipeline;
    }

    void Shader::CreateDescriptorSets(Vector<VkDescriptorSet>& descriptor_sets, Vector<UniformSet>& uniform_sets)
    {
        if (m_uniform_sets.Size() > 0)
        {
            Display::Instance()->CreateDescriptorSets(
                m_uniform_sets,
                m_descriptor_pool,
                m_descriptor_layouts,
                descriptor_sets);
            uniform_sets = m_uniform_sets;
        }
    }
#elif VR_GLES
    static String ProcessShaderSource(const String& glsl, const String& predefine, const Vector<String>& includes)
    {
#if VR_MAC
        String shader_header =
            "#version 120\n"
            "#define precision\n"
            "#define highp\n"
            "#define mediump\n"
            "#define lowp\n";
#else
        String shader_header = "";
#endif

#if VR_WINDOWS
        if (!predefine.StartsWith("#version"))
        {
            shader_header = "#version 120\n";
        }
#endif

        String source = shader_header;
        source += predefine + "\n";

        for (const auto& i : includes)
        {
            auto include_path = Application::Instance()->GetDataPath() + "/shader/Include/" + i;
            auto bytes = File::ReadAllBytes(include_path);
            auto include_str = String(bytes);
            source += include_str + "\n";
        }
        source += glsl;

        return source;
    }

    static GLuint CompileShader(const String& source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        const char* str = source.CString();
        glShaderSource(shader, 1, &str, nullptr);
        glCompileShader(shader);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            int log_size = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
            if (log_size > 1)
            {
                ByteBuffer buffer(log_size);
                glGetShaderInfoLog(shader, log_size, nullptr, (GLchar*) buffer.Bytes());
                Log("shader compile error: %s", buffer.Bytes());

                glDeleteShader(shader);
                shader = 0;
            }
        }

        return shader;
    }

    void Shader::CreateProgram(
        const String& vs_predefine,
        const Vector<String>& vs_includes,
        const String& vs_source,
        const String& fs_predefine,
        const Vector<String>& fs_includes,
        const String& fs_source)
    {
        GLuint vs = CompileShader(ProcessShaderSource(vs_source, vs_predefine, vs_includes), GL_VERTEX_SHADER);
        GLuint fs = CompileShader(ProcessShaderSource(fs_source, fs_predefine, fs_includes), GL_FRAGMENT_SHADER);

        if (vs && fs)
        {
            GLuint program = glCreateProgram();
            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);

            int success = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success)
            {
                int log_size = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
                if (log_size > 1)
                {
                    ByteBuffer buffer(log_size);
                    glGetProgramInfoLog(program, log_size, nullptr, (GLchar*) buffer.Bytes());
                    Log("program link error: %s", buffer.Bytes());

                    glDeleteProgram(program);
                }
            }
            else
            {
                const int name_size = 1024;
                char name[name_size];

                int attribute_count = 0;
                glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attribute_count);
                for (int i = 0; i < attribute_count; ++i)
                {
                    Attribute a;
                    glGetActiveAttrib(program, i, name_size, nullptr, &a.size, &a.type, name);
                    a.name = name;
                    a.loc = glGetAttribLocation(program, a.name.CString());

                    switch (a.type)
                    {
                    case GL_FLOAT_VEC2:
                        a.vector_size = 2;
                        break;
                    case GL_FLOAT_VEC3:
                        a.vector_size = 3;
                        break;
                    case GL_FLOAT_VEC4:
                        a.vector_size = 4;
                        break;
                    default:
                        assert(!"invalid vertex attribute vector size");
                        break;
                    }

                    m_attributes.Add(a);
                }

                int uniform_count = 0;
                glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

                for (int i = 0; i < uniform_count; ++i)
                {
                    Uniform u;
                    glGetActiveUniform(program, i, name_size, nullptr, &u.size, &u.type, name);
                    u.name = name;
                    // array maybe ends with '[0]'
                    if (u.size > 1 && u.name.EndsWith("[0]"))
                    {
                        u.name = u.name.Substring(0, u.name.Size() - 3);
                    }
                    u.loc = glGetUniformLocation(program, u.name.CString());

                    m_uniforms.Add(u);
                }

                m_program = program;
            }
        }
       
        if (vs)
        {
            glDeleteShader(vs);
        }
        if (fs)
        {
            glDeleteShader(fs);
        }
    }

    bool Shader::Use() const
    {
        if (m_program)
        {
            glUseProgram(m_program);
            return true;
        }
        else
        {
            return false;
        }
    }

    void Shader::EnableVertexAttribs() const
    {
        for (int i = 0; i < (int) VertexAttributeType::Count; ++i)
        {
            int loc = glGetAttribLocation(m_program, VERTEX_ATTR_NAMES[i]);
            if (loc >= 0)
            {
                glEnableVertexAttribArray(loc);
                glVertexAttribPointer(loc, VERTEX_ATTR_SIZES[i] / 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*) (size_t) VERTEX_ATTR_OFFSETS[i]);
            }
        }
    }

    void Shader::DisableVertexAttribs() const
    {
        for (int i = 0; i < (int) VertexAttributeType::Count; ++i)
        {
            int loc = glGetAttribLocation(m_program, VERTEX_ATTR_NAMES[i]);
            if (loc >= 0)
            {
                glDisableVertexAttribArray(loc);
            }
        }
    }

    void Shader::SetUniform1f(const String& name, float value) const
    {
        for (int i = 0; i < m_uniforms.Size(); ++i)
        {
            const Uniform& u = m_uniforms[i];
            if (u.name == name)
            {
                glUniform1f(u.loc, value);
                break;
            }
        }
    }

    void Shader::SetUniform4f(const String& name, int count, const float* value) const
    {
        for (int i = 0; i < m_uniforms.Size(); ++i)
        {
            const Uniform& u = m_uniforms[i];
            if (u.name == name)
            {
                glUniform4fv(u.loc, count, value);
                break;
            }
        }
    }

    void Shader::SetUniform1i(const String& name, int value) const
    {
        for (int i = 0; i < m_uniforms.Size(); ++i)
        {
            const Uniform& u = m_uniforms[i];
            if (u.name == name)
            {
                glUniform1i(u.loc, value);
                break;
            }
        }
    }

    void Shader::SetUniformMatrix(const String& name, int count, const float* value) const
    {
        for (int i = 0; i < m_uniforms.Size(); ++i)
        {
            const Uniform& u = m_uniforms[i];
            if (u.name == name)
            {
                glUniformMatrix4fv(u.loc, count, GL_FALSE, value);
                break;
            }
        }
    }

    void Shader::ApplyRenderState()
    {
        if (m_render_state.cull == RenderState::Cull::Off)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace((GLenum) m_render_state.cull);
        }

        if (m_render_state.zTest == RenderState::ZTest::Off)
        {
            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc((GLenum) m_render_state.zTest);
        }

        glDepthMask((GLboolean) m_render_state.zWrite);

        if (m_render_state.blend == RenderState::Blend::Off)
        {
            glDisable(GL_BLEND);
        }
        else
        {
            glEnable(GL_BLEND);
            glBlendFunc((GLenum) m_render_state.srcBlendMode, (GLenum) m_render_state.dstBlendMode);
        }
    }
#endif
}
