/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#if VR_GLES

#include "ShaderGLES.h"
#include "MaterialGLES.h"
#include "Application.h"
#include "graphics/Shader.h"
#include "graphics/UniformBuffer.h"
#include "graphics/XMLShader.h"
#include "graphics/Texture2D.h"
#include "graphics/Camera.h"
#include "graphics/Graphics.h"
#include "graphics/RenderPass.h"
#include "graphics/Material.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "memory/Memory.h"
#include "Debug.h"

namespace Viry3D
{
	static const int UNIFORM_BUFFER_OBJ_BINDING = 0;

	static GLuint create_shader(GLenum type, const String& src)
	{
		LogGLError();

		auto shader = glCreateShader(type);

		auto source = src.CString();
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				ByteBuffer buffer(len);
				glGetShaderInfoLog(shader, len, NULL, (GLchar*) buffer.Bytes());
				Log("shader compile error:\n%s", buffer.Bytes());
			}

			glDeleteShader(shader);
			shader = 0;
		}

		LogGLError();

		return shader;
	}

	static GLuint create_program(GLuint vs, GLuint ps)
	{
		LogGLError();

		auto program = glCreateProgram();

		glAttachShader(program, vs);
		glAttachShader(program, ps);

		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLint len;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				ByteBuffer buffer(len);
				glGetProgramInfoLog(program, len, NULL, (GLchar*) buffer.Bytes());
				Log("shader link error:\n%s", buffer.Bytes());
			}

			glDeleteProgram(program);
			program = 0;
		}

		LogGLError();

		return program;
	}

	static void prepare_pipeline(
		const XMLPass& pass,
		XMLShader& xml,
		ShaderPass& shader_pass)
	{
		LogGLError();

		auto display = (DisplayGLES*) Graphics::GetDisplay();
		auto program = shader_pass.program;

		auto push_index = glGetUniformBlockIndex(program, "buf_vs_obj");
		if (push_index != 0xffffffff)
		{
			glUniformBlockBinding(program, push_index, UNIFORM_BUFFER_OBJ_BINDING);
		}
		else
		{
			Log("no world uniform in shader:%s", xml.name.CString());
		}

		auto& uniform_buffer_infos = shader_pass.uniform_buffer_infos;
		for (auto& i : xml.vss)
		{
			if (i.name == pass.vs)
			{
				if (i.uniform_buffer.binding >= 0)
				{
					uniform_buffer_infos.Add(&i.uniform_buffer);
				}

				shader_pass.vs = &i;
				break;
			}
		}
		for (auto& i : xml.pss)
		{
			if (i.name == pass.ps)
			{
				if (i.uniform_buffer.binding >= 0)
				{
					uniform_buffer_infos.Add(&i.uniform_buffer);
				}
				break;
			}
		}

		auto& sampler_infos = shader_pass.sampler_infos;
		for (const auto& i : xml.pss)
		{
			if (i.name == pass.ps)
			{
				for (auto& j : i.samplers)
				{
					sampler_infos.Add(&j);
				}
				break;
			}
		}

		int buffer_size = 0;
		for (auto& i : uniform_buffer_infos)
		{
			auto index = glGetUniformBlockIndex(program, i->name.CString());
			glUniformBlockBinding(program, index, i->binding);

			GLint size;
			glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
			assert(size == i->size);

			int offset = buffer_size;
			int offset_alignment = display->GetMinUniformBufferOffsetAlignment();
			if (offset % offset_alignment != 0)
			{
				offset += offset_alignment - (offset % offset_alignment);
			}
			i->offset = offset;
			buffer_size += i->size;

			for (auto& j : i->uniforms)
			{
				auto name = i->name + "." + j.name;
				const char* names[] = { name.CString() };

				GLuint index;
				glGetUniformIndices(program, 1, names, &index);
				GLint offset;
				glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_OFFSET, &offset);

				//Log("%s uniform:%s index:%d offset:%d", xml.name.CString(), name.CString(), index, offset);
			}
		}

		for (auto& i : sampler_infos)
		{
			auto location = glGetUniformLocation(program, i->name.CString());

			shader_pass.sampler_locations.Add(location);
		}

		const XMLRenderState *prs = NULL;
		for (const auto& i : xml.rss)
		{
			if (i.name == pass.rs)
			{
				prs = &i;
				break;
			}
		}

		assert(prs != NULL);

		GLRenderState& state = shader_pass.render_state;

		state.offset_enable = prs->Offset.enable;
		if (prs->Offset.enable)
		{
			state.offset_factor = prs->Offset.factor;
			state.offset_units = prs->Offset.units;
		}

		if (prs->Cull == "Off")
		{
			state.cull_enable = false;
		}
		else
		{
			state.cull_enable = true;

			if (prs->Cull == "Back")
			{
				state.cull_face = GL_BACK;
			}
			else if (prs->Cull == "Front")
			{
				state.cull_face = GL_FRONT;
			}
		}

		{
			state.color_mask_r = GL_FALSE;
			state.color_mask_g = GL_FALSE;
			state.color_mask_b = GL_FALSE;
			state.color_mask_a = GL_FALSE;

			if (prs->ColorMask.Contains("R"))
			{
				state.color_mask_r = GL_TRUE;
			}
			if (prs->ColorMask.Contains("G"))
			{
				state.color_mask_g = GL_TRUE;
			}
			if (prs->ColorMask.Contains("B"))
			{
				state.color_mask_b = GL_TRUE;
			}
			if (prs->ColorMask.Contains("A"))
			{
				state.color_mask_a = GL_TRUE;
			}
		}

		state.blend_enable = prs->Blend.enable;
		if (prs->Blend.enable)
		{
			const char* strs[] = {
				"One",
				"Zero",
				"SrcColor",
				"SrcAlpha",
				"DstColor",
				"DstAlpha",
				"OneMinusSrcColor",
				"OneMinusSrcAlpha",
				"OneMinusDstColor",
				"OneMinusDstAlpha"
			};
			const int values[] = {
				GL_ONE,
				GL_ZERO,
				GL_SRC_COLOR,
				GL_SRC_ALPHA,
				GL_DST_COLOR,
				GL_DST_ALPHA,
				GL_ONE_MINUS_SRC_COLOR,
				GL_ONE_MINUS_SRC_ALPHA,
				GL_ONE_MINUS_DST_COLOR,
				GL_ONE_MINUS_DST_ALPHA
			};

			const int count = sizeof(values) / sizeof(values[0]);
			for (int i = 0; i < count; i++)
			{
				if (prs->Blend.src == strs[i])
				{
					state.blend_src_c = values[i];
				}

				if (prs->Blend.dst == strs[i])
				{
					state.blend_dst_c = values[i];
				}

				if (prs->Blend.src_a == strs[i])
				{
					state.blend_src_a = values[i];
				}

				if (prs->Blend.dst_a == strs[i])
				{
					state.blend_dst_a = values[i];
				}
			}
		}

		state.depth_mask = prs->ZWrite == "On" ? GL_TRUE : GL_FALSE;

		{
			const char* strs[] = {
				"Less",
				"Greater",
				"LEqual",
				"GEqual",
				"Equal",
				"NotEqual",
				"Always"
			};
			const int values[] = {
				GL_LESS,
				GL_GREATER,
				GL_LEQUAL,
				GL_GEQUAL,
				GL_EQUAL,
				GL_NOTEQUAL,
				GL_ALWAYS
			};

			const int count = sizeof(values) / sizeof(values[0]);
			for (int i = 0; i < count; i++)
			{
				if (prs->ZTest == strs[i])
				{
					state.depth_func = values[i];
					break;
				}
			}
		}

		state.stencil_enable = prs->Stencil.enable;
		if (prs->Stencil.enable)
		{
			state.stencil_ref = prs->Stencil.RefValue;
			state.stencil_read_mask = prs->Stencil.ReadMask;
			state.stencil_write_mask = prs->Stencil.WriteMask;

			{
				const char* strs[] = {
					"Less",
					"Greater",
					"LEqual",
					"GEqual",
					"Equal",
					"NotEqual",
					"Always",
					"Never"
				};
				const int values[] = {
					GL_LESS,
					GL_GREATER,
					GL_LEQUAL,
					GL_GEQUAL,
					GL_EQUAL,
					GL_NOTEQUAL,
					GL_ALWAYS,
					GL_NEVER
				};

				const int count = sizeof(values) / sizeof(values[0]);
				for (int i = 0; i < count; i++)
				{
					if (prs->Stencil.Comp == strs[i])
					{
						state.stencil_func = values[i];
						break;
					}
				}
			}

			{
				const char* strs[] = {
					"Keep",
					"Zero",
					"Replace",
					"IncrSat",
					"DecrSat",
					"Invert",
					"IncrWrap",
					"DecrWrap"
				};
				const int values[] = {
					GL_KEEP,
					GL_ZERO,
					GL_REPLACE,
					GL_INCR,
					GL_DECR,
					GL_INVERT,
					GL_INCR_WRAP,
					GL_DECR_WRAP,
				};

				const int count = sizeof(values) / sizeof(values[0]);
				for (int i = 0; i < count; i++)
				{
					if (prs->Stencil.Pass == strs[i])
					{
						state.stencil_op_pass = values[i];
					}

					if (prs->Stencil.Fail == strs[i])
					{
						state.stencil_op_fail = values[i];
					}

					if (prs->Stencil.ZFail == strs[i])
					{
						state.stencil_op_zfail = values[i];
					}
				}
			}
		}

		LogGLError();
	}

	ShaderGLES::ShaderGLES()
	{
	}

	ShaderGLES::~ShaderGLES()
	{
		for (auto& i : m_passes)
		{
			glDeleteProgram(i.program);
		}
		m_passes.Clear();

		for (auto& i : m_vertex_shaders)
		{
			glDeleteShader(i.second);
		}
		m_vertex_shaders.Clear();

		for (auto& i : m_pixel_shaders)
		{
			glDeleteShader(i.second);
		}
		m_pixel_shaders.Clear();
	}

	void ShaderGLES::Compile()
	{
		this->CreateShaders();
		this->CreatePasses();
	}

	const String g_shader_header =
		"#version 300 es\n"
		"#define VR_GLES 1\n"
		"#define UniformBuffer(set_index, binding_index) layout(std140)\n"
		"#define UniformTexture(set_index, binding_index)\n"
		"#define Varying(location_index)\n";

	static String combine_shader_src(const Vector<String>& includes, const String& src)
	{
		String source = g_shader_header;
		for (const auto& i : includes)
		{
			auto include_path = Application::DataPath() + "/shader/Include/" + i;
			auto bytes = File::ReadAllBytes(include_path);
			auto include_str = String(bytes);
			source += include_str + "\n";
		}
		source += src;

		return source;
	}

	void ShaderGLES::CreateShaders()
	{
		const auto& xml = ((Shader*) this)->m_xml;

		for (const auto& i : xml.vss)
		{
			auto source = combine_shader_src(i.includes, i.src);

			auto shader = create_shader(GL_VERTEX_SHADER, source);

			m_vertex_shaders.Add(i.name, shader);
		}

		for (const auto& i : xml.pss)
		{
			auto source = combine_shader_src(i.includes, i.src);

			auto shader = create_shader(GL_FRAGMENT_SHADER, source);

			m_pixel_shaders.Add(i.name, shader);
		}
	}

	void ShaderGLES::CreatePasses()
	{
		auto& xml = ((Shader*) this)->m_xml;

		m_passes.Resize(xml.passes.Size());
		for (int i = 0; i < xml.passes.Size(); i++)
		{
			auto& xml_pass = xml.passes[i];
			auto& pass = m_passes[i];

			pass.name = xml_pass.name;
			pass.program = create_program(m_vertex_shaders[xml_pass.vs], m_pixel_shaders[xml_pass.ps]);

			prepare_pipeline(xml_pass, xml, pass);
		}
	}

	Ref<UniformBuffer> ShaderGLES::CreateUniformBuffer(int index)
	{
		Ref<UniformBuffer> uniform_buffer;

		auto& pass = m_passes[index];
		auto& uniform_buffer_infos = pass.uniform_buffer_infos;

		int buffer_size = 0;
		for (int i = uniform_buffer_infos.Size() - 1; i >= 0; i--)
		{
			buffer_size = uniform_buffer_infos[i]->offset + uniform_buffer_infos[i]->size;
			break;
		}

		if (buffer_size > 0)
		{
			uniform_buffer = UniformBuffer::Create(buffer_size);
		}

		return uniform_buffer;
	}

	void ShaderGLES::BeginPass(int index)
	{
		LogGLError();

		auto& rs = m_passes[index].render_state;
		if (rs.offset_enable)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(rs.offset_factor, rs.offset_units);
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		if (rs.cull_enable)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(rs.cull_face);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glColorMask(rs.color_mask_r, rs.color_mask_g, rs.color_mask_b, rs.color_mask_a);

		if (rs.blend_enable)
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(rs.blend_src_c, rs.blend_dst_c, rs.blend_src_a, rs.blend_dst_a);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		glDepthMask(rs.depth_mask);
		glDepthFunc(rs.depth_func);

		if (rs.stencil_enable)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(rs.stencil_func, rs.stencil_ref, rs.stencil_read_mask);
			glStencilMask(rs.stencil_write_mask);
			glStencilOp(rs.stencil_op_fail, rs.stencil_op_zfail, rs.stencil_op_pass);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}

		int width = RenderPass::GetRenderPassBinding()->GetFrameBufferWidth();
		int height = RenderPass::GetRenderPassBinding()->GetFrameBufferHeight();
		auto rect = RenderPass::GetRenderPassBinding()->GetRect();

		int viewport_x = (int) (rect.x * width);
		int viewport_y = (int) (rect.y * height);
		int viewport_width = (int) (rect.width * width);
		int viewport_height = (int) (rect.height * height);

		glViewport(viewport_x, viewport_y, viewport_width, viewport_height);

		glUseProgram(m_passes[index].program);

		LogGLError();
	}

	void ShaderGLES::BindSharedMaterial(int index, const Ref<Material>& material)
	{
		material->Apply(index);
	}

	void ShaderGLES::UpdateRendererDescriptorSet(Ref<DescriptorSet>& renderer_descriptor_set, Ref<UniformBuffer>& descriptor_set_buffer, const void* data, int size, int lightmap_index)
	{
		if (!descriptor_set_buffer)
		{
			descriptor_set_buffer = UniformBuffer::Create(size);
		}

		// update buffer
		descriptor_set_buffer->UpdateRange(0, size, data);
	}

	void ShaderGLES::BindRendererDescriptorSet(int index, const Ref<Material>& material, Ref<UniformBuffer>& descriptor_set_buffer, int lightmap_index)
	{
		LogGLError();

		glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BUFFER_OBJ_BINDING, descriptor_set_buffer->GetBuffer());

		if (lightmap_index >= 0)
		{
			material->ApplyLightmap(index, lightmap_index);
		}

		LogGLError();
	}
}

#endif
