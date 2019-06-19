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
#include "Debug.h"
#include "Engine.h"
#include "io/File.h"
#include "lua/lua.hpp"
#include "memory/Memory.h"

#if VR_VULKAN || VR_D3D
#include "vulkan/spirv_shader_compiler.h"
#endif

#if VR_D3D
#include "vulkan/spirv_cross/spirv_hlsl.hpp"
#endif

#if VR_METAL
#include "GLSLConversion.h"
#include "SPIRVConversion.h"
#endif

namespace Viry3D
{
	Map<String, Ref<Shader>> Shader::m_shaders;

#if VR_VULKAN || VR_D3D
    static void GlslToSpirv(const String& glsl, ShaderCompiler::ShaderType shader_type, Vector<unsigned int>& spirv)
    {
#if VR_WINDOWS || VR_ANDROID || VR_UWP
        String error;
        bool success = GlslToSpv(shader_type, glsl.CString(), spirv, error);
        if (!success)
        {
            Log("shader compile error: %s", error.CString());
        }
        assert(success);
#elif VR_MAC || VR_IOS
        MVKShaderStage stage;
        switch (shader_type)
        {
		case ShaderCompiler::ShaderType::Vertex:
            stage = kMVKShaderStageVertex;
            break;
        case ShaderCompiler::ShaderType::Fragment:
            stage = kMVKShaderStageFragment;
            break;
        default:
            stage = kMVKShaderStageAuto;
            break;
        }
        uint32_t* spirv_code = nullptr;
        size_t size = 0;
        char* log = nullptr;
        bool success = mvkConvertGLSLToSPIRV(glsl.CString(),
                                             stage,
                                             &spirv_code,
                                             &size,
                                             &log,
                                             true,
                                             true);
        if (!success)
        {
            Log("shader compile error: %s", log);
        }
        assert(success);
        
        spirv.Resize((int) size / 4);
        Memory::Copy(&spirv[0], spirv_code, spirv.SizeInBytes());
        
        free(log);
        free(spirv_code);
#endif
    }
#endif
    
    void Shader::Init()
    {
#if VR_VULKAN || VR_D3D
		ShaderCompiler::InitShaderCompiler();
#endif
    }
    
    void Shader::Done()
    {
		m_shaders.Clear();

#if VR_VULKAN || VR_D3D
		ShaderCompiler::DeinitShaderCompiler();
#endif
    }

	Ref<Shader> Shader::Find(const String& name, const Vector<String>& keywords, bool light_add)
	{
		Ref<Shader> shader;

		List<String> keyword_list;
		for (int i = 0; i < keywords.Size(); ++i)
		{
			keyword_list.AddLast(keywords[i]);
		}
		keyword_list.Sort();

		String key = name;
		for (const auto& i : keyword_list)
		{
			key += "|" + i;
		}

		Ref<Shader>* find;
		if (m_shaders.TryGet(key, &find))
		{
			shader = *find;
		}
		else
		{
			String path = Engine::Instance()->GetDataPath() + "/shader/" + name + ".lua";
			if (File::Exist(path))
			{
				String lua_src = File::ReadAllText(path);

				shader = Ref<Shader>(new Shader(name, light_add));
				shader->Load(lua_src, keyword_list);
				shader->Compile();

				m_shaders.Add(key, shader);
			}
			else
			{
				Log("Shader %s not exist: %s", name.CString(), path.CString());
			}
		}

		return shader;
	}
    
    Shader::Shader(const String& name, bool light_add):
		m_light_add(light_add),
		m_queue(0)
    {
        this->SetName(name);
    }
    
    Shader::~Shader()
    {
		auto& driver = Engine::Instance()->GetDriverApi();

		for (int i = 0; i < m_passes.Size(); ++i)
		{
			auto& pass = m_passes[i];

			if (pass.pipeline.program)
			{
				driver.destroyProgram(pass.pipeline.program);
				pass.pipeline.program.clear();
			}
		}
		m_passes.Clear();
    }

	static void SetGlobalInt(lua_State* L, const char* key, int value)
	{
		lua_pushinteger(L, value);
		lua_setglobal(L, key);
	}

	static void AddLuaPath(lua_State* L, const String& path)
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
		String cur_path = lua_tostring(L, -1); // grab path string from top of stack
		cur_path += ";" + path; // do your path magic here
		lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
		lua_pushstring(L, cur_path.CString()); // push the new one
		lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
		lua_pop(L, 1); // get rid of package table from top of stack
	}

	static void GetTableString(lua_State* L, const char* key, String& str)
	{
		lua_pushstring(L, key);
		lua_gettable(L, -2);
		if (lua_isstring(L, -1))
		{
			str = lua_tostring(L, -1);
		}
		lua_pop(L, 1);
	}

	template <class T>
	static void GetTableInt(lua_State* L, const char* key, T& num)
	{
		lua_pushstring(L, key);
		lua_gettable(L, -2);
		if (lua_isinteger(L, -1))
		{
			num = (T) lua_tointeger(L, -1);
		}
		lua_pop(L, 1);
	}

	void Shader::Load(const String& src, const List<String>& keywords)
	{
		m_keywords = keywords;

		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		SetGlobalInt(L, "Off", 0);
		SetGlobalInt(L, "On", 1);

		SetGlobalInt(L, "Back", (int) filament::backend::CullingMode::BACK);
		SetGlobalInt(L, "Front", (int) filament::backend::CullingMode::FRONT);
		
		SetGlobalInt(L, "Less", (int) filament::backend::SamplerCompareFunc::L);
		SetGlobalInt(L, "Greater", (int) filament::backend::SamplerCompareFunc::G);
		SetGlobalInt(L, "LEqual", (int) filament::backend::SamplerCompareFunc::LE);
		SetGlobalInt(L, "GEqual", (int) filament::backend::SamplerCompareFunc::GE);
		SetGlobalInt(L, "Equal", (int) filament::backend::SamplerCompareFunc::E);
		SetGlobalInt(L, "NotEqual", (int) filament::backend::SamplerCompareFunc::NE);
		SetGlobalInt(L, "Always", (int) filament::backend::SamplerCompareFunc::A);

		SetGlobalInt(L, "Zero", (int) filament::backend::BlendFunction::ZERO);
		SetGlobalInt(L, "One", (int) filament::backend::BlendFunction::ONE);
		SetGlobalInt(L, "SrcColor", (int) filament::backend::BlendFunction::SRC_COLOR);
		SetGlobalInt(L, "SrcAlpha", (int) filament::backend::BlendFunction::SRC_ALPHA);
		SetGlobalInt(L, "DstColor", (int) filament::backend::BlendFunction::DST_COLOR);
		SetGlobalInt(L, "DstAlpha", (int) filament::backend::BlendFunction::DST_ALPHA);
		SetGlobalInt(L, "OneMinusSrcColor", (int) filament::backend::BlendFunction::ONE_MINUS_SRC_COLOR);
		SetGlobalInt(L, "OneMinusSrcAlpha", (int) filament::backend::BlendFunction::ONE_MINUS_SRC_ALPHA);
		SetGlobalInt(L, "OneMinusDstColor", (int) filament::backend::BlendFunction::ONE_MINUS_DST_COLOR);
		SetGlobalInt(L, "OneMinusDstAlpha", (int) filament::backend::BlendFunction::ONE_MINUS_DST_ALPHA);

		SetGlobalInt(L, "Background", (int) Queue::Background);
		SetGlobalInt(L, "Geometry", (int) Queue::Geometry);
		SetGlobalInt(L, "AlphaTest", (int) Queue::AlphaTest);
		SetGlobalInt(L, "Transparent", (int) Queue::Transparent);
		SetGlobalInt(L, "Overlay", (int) Queue::Overlay);

		SetGlobalInt(L, "None", (int) LightMode::None);
		SetGlobalInt(L, "Forward", (int) LightMode::Forward);

		String dir = Engine::Instance()->GetDataPath() + "/shader/" + this->GetName();
		dir = dir.Substring(0, dir.LastIndexOf("/"));
		AddLuaPath(L, dir + "/?.lua");

		if (luaL_dostring(L, src.CString()) != 0)
		{
			String error = lua_tostring(L, -1);
			lua_pop(L, 1);
			Log("do lua error: %s\n", error.CString());
			assert(false);
		}

		if (lua_istable(L, -1))
		{
			// pass array
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				if (lua_istable(L, -1))
				{
					// pass
					Pass pass;
					GetTableString(L, "vs", pass.vs);
					GetTableString(L, "fs", pass.fs);

					lua_pushstring(L, "rs");
					lua_gettable(L, -2);
					if (lua_istable(L, -1))
					{
						// rs
						filament::backend::CullingMode culling = filament::backend::CullingMode::BACK;
						GetTableInt(L, "Cull", culling);
						pass.pipeline.rasterState.culling = culling;

						filament::backend::SamplerCompareFunc depth_func = filament::backend::SamplerCompareFunc::LE;
						GetTableInt(L, "ZTest", depth_func);
						pass.pipeline.rasterState.depthFunc = depth_func;

						bool depth_write = true;
						GetTableInt(L, "ZWrite", depth_write);
						pass.pipeline.rasterState.depthWrite = depth_write;

						filament::backend::BlendFunction src_blend = filament::backend::BlendFunction::ONE;
						filament::backend::BlendFunction dst_blend = filament::backend::BlendFunction::ZERO;
						GetTableInt(L, "SrcBlendMode", src_blend);
						GetTableInt(L, "DstBlendMode", dst_blend);
						pass.pipeline.rasterState.blendFunctionSrcRGB = src_blend;
						pass.pipeline.rasterState.blendFunctionSrcAlpha = src_blend;
						pass.pipeline.rasterState.blendFunctionDstRGB = dst_blend;
						pass.pipeline.rasterState.blendFunctionDstAlpha = dst_blend;

						bool color_write = true;
						GetTableInt(L, "CWrite", color_write);
						pass.pipeline.rasterState.colorWrite = color_write;

						GetTableInt(L, "Queue", pass.queue);

						if (m_queue < pass.queue)
						{
							m_queue = pass.queue;
						}

						GetTableInt(L, "LightMode", pass.light_mode);

						if (pass.light_mode == LightMode::Forward && m_light_add)
						{
							pass.pipeline.rasterState.depthWrite = false;
							pass.pipeline.rasterState.blendFunctionSrcRGB = src_blend;
							pass.pipeline.rasterState.blendFunctionSrcAlpha = src_blend;
							pass.pipeline.rasterState.blendFunctionDstRGB = filament::backend::BlendFunction::ONE;
							pass.pipeline.rasterState.blendFunctionDstAlpha = filament::backend::BlendFunction::ONE;
						}
					}
					lua_pop(L, 1);

					lua_pushstring(L, "uniforms");
					lua_gettable(L, -2);
					if (lua_istable(L, -1))
					{
						lua_pushnil(L);
						while (lua_next(L, -2))
						{
							if (lua_istable(L, -1))
							{
								Uniform uniform;
								GetTableString(L, "name", uniform.name);
								GetTableInt(L, "binding", uniform.binding);

								int offset = 0;

								lua_pushstring(L, "members");
								lua_gettable(L, -2);
								if (lua_istable(L, -1))
								{
									lua_pushnil(L);
									while (lua_next(L, -2))
									{
										if (lua_istable(L, -1))
										{
											Member member;
											GetTableString(L, "name", member.name);
											GetTableInt(L, "size", member.size);

											member.offset = offset;
											offset += member.size;

											uniform.members.Add(member);
										}
										lua_pop(L, 1);
									}
								}
								lua_pop(L, 1);

								uniform.size = offset;

								pass.uniforms.Add(uniform);
							}
							lua_pop(L, 1);
						}
					}
					lua_pop(L, 1);

					lua_pushstring(L, "samplers");
					lua_gettable(L, -2);
					if (lua_istable(L, -1))
					{
						lua_pushnil(L);
						while (lua_next(L, -2))
						{
							if (lua_istable(L, -1))
							{
								SamplerGroup group;
								GetTableString(L, "name", group.name);
								GetTableInt(L, "binding", group.binding);

								lua_pushstring(L, "samplers");
								lua_gettable(L, -2);
								if (lua_istable(L, -1))
								{
									lua_pushnil(L);
									while (lua_next(L, -2))
									{
										if (lua_istable(L, -1))
										{
											Sampler sampler;
											GetTableString(L, "name", sampler.name);
											GetTableInt(L, "binding", sampler.binding);

											group.samplers.Add(sampler);
										}
										lua_pop(L, 1);
									}
								}
								lua_pop(L, 1);

								pass.samplers.Add(group);
							}
							lua_pop(L, 1);
						}
					}
					lua_pop(L, 1);

					m_passes.Add(pass);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		lua_close(L);
	}

	void Shader::Compile()
	{
		auto& driver = Engine::Instance()->GetDriverApi();

#if VR_WINDOWS || VR_MAC
		String version = "#version 410\n";
#else
		String version = "#version 300 es\n";
		if (Engine::Instance()->GetBackend() == filament::backend::Backend::VULKAN ||
            Engine::Instance()->GetBackend() == filament::backend::Backend::METAL ||
			Engine::Instance()->GetBackend() == filament::backend::Backend::D3D11)
		{
			version = "#version 310 es\n";
		}
#endif
		
		String define;
		String vk_convert;

		if (Engine::Instance()->GetBackend() == filament::backend::Backend::VULKAN ||
            Engine::Instance()->GetBackend() == filament::backend::Backend::METAL ||
			Engine::Instance()->GetBackend() == filament::backend::Backend::D3D11)
		{
			define = "#extension GL_ARB_separate_shader_objects : enable\n"
				"#extension GL_ARB_shading_language_420pack : enable\n"
				"#define VK_LAYOUT_LOCATION(i) layout(location = i)\n"
				"#define VK_UNIFORM_BINDING(i) layout(std140, set = 0, binding = i)\n"
				"#define VK_SAMPLER_BINDING(i) layout(set = 1, binding = i)\n";
			if (Engine::Instance()->GetBackend() == filament::backend::Backend::VULKAN ||
				Engine::Instance()->GetBackend() == filament::backend::Backend::METAL)
			{
				vk_convert = "void vk_convert() {\n"
					"gl_Position.y = -gl_Position.y;\n"
					"gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
					"}\n";
			}
			else
			{
				vk_convert = "void vk_convert() { }\n";
			}
		}
		else if (Engine::Instance()->GetBackend() == filament::backend::Backend::OPENGL)
		{
			define = "#define VR_GLES 1\n"
				"#define VK_LAYOUT_LOCATION(i)\n"
				"#define VK_UNIFORM_BINDING(i) layout(std140)\n"
				"#define VK_SAMPLER_BINDING(i)\n";
			vk_convert = "void vk_convert() { }\n";
		}
		else
		{
			assert(false);
		}

		for (const auto& i : m_keywords)
		{
			define += "#define " + i + " 1\n";
		}

		for (int i = 0; i < m_passes.Size(); ++i)
		{
			auto& pass = m_passes[i];

			String vs = version + define + vk_convert + pass.vs;
			String fs = version + define + pass.fs;
			
			Vector<char> vs_data;
			Vector<char> fs_data;

            if (Engine::Instance()->GetBackend() == filament::backend::Backend::VULKAN)
            {
#if VR_VULKAN
                Vector<unsigned int> vs_spirv;
                Vector<unsigned int> fs_spirv;
                GlslToSpirv(vs, ShaderCompiler::ShaderType::Vertex, vs_spirv);
                GlslToSpirv(fs, ShaderCompiler::ShaderType::Fragment, fs_spirv);

                vs_data.Resize(vs_spirv.Size() * 4);
                Memory::Copy(&vs_data[0], &vs_spirv[0], vs_data.Size());
                fs_data.Resize(fs_spirv.Size() * 4);
                Memory::Copy(&fs_data[0], &fs_spirv[0], fs_data.Size());
#endif
            }
			else if (Engine::Instance()->GetBackend() == filament::backend::Backend::D3D11)
			{
#if VR_D3D
				Vector<unsigned int> vs_spirv;
				Vector<unsigned int> fs_spirv;
				GlslToSpirv(vs, ShaderCompiler::ShaderType::Vertex, vs_spirv);
				GlslToSpirv(fs, ShaderCompiler::ShaderType::Fragment, fs_spirv);

                spirv_cross::CompilerHLSL::Options options;
                options.shader_model = 40;

                spirv_cross::CompilerHLSL vs_compiler(&vs_spirv[0], vs_spirv.Size());
                vs_compiler.set_hlsl_options(options);
                std::string vs_hlsl = vs_compiler.compile();

                spirv_cross::CompilerHLSL fs_compiler(&fs_spirv[0], fs_spirv.Size());
                fs_compiler.set_hlsl_options(options);
                std::string fs_hlsl = fs_compiler.compile();

                vs_data.Resize((int) vs_hlsl.size());
                Memory::Copy(&vs_data[0], &vs_hlsl[0], (int) vs_hlsl.size());
                fs_data.Resize((int) fs_hlsl.size());
                Memory::Copy(&fs_data[0], &fs_hlsl[0], (int) fs_hlsl.size());
#endif
			}
            else if (Engine::Instance()->GetBackend() == filament::backend::Backend::METAL)
            {
#if VR_METAL
                auto glsl_to_spirv = [](const String& glsl, filament::backend::Program::Shader shader_type, Vector<unsigned int>& spirv) {
                    MVKShaderStage stage;
                    switch (shader_type)
                    {
                        case filament::backend::Program::Shader::VERTEX:
                            stage = kMVKShaderStageVertex;
                            break;
                        case filament::backend::Program::Shader::FRAGMENT:
                            stage = kMVKShaderStageFragment;
                            break;
                        default:
                            stage = kMVKShaderStageAuto;
                            break;
                    }
                    uint32_t* spirv_code = nullptr;
                    size_t size = 0;
                    char* log = nullptr;
                    bool success = mvkConvertGLSLToSPIRV(glsl.CString(),
                                                         stage,
                                                         &spirv_code,
                                                         &size,
                                                         &log,
                                                         true,
                                                         true);
                    if (!success)
                    {
                        Log("shader compile error: %s", log);
                    }
                    assert(success);
                    
                    spirv.Resize((int) size / 4);
                    Memory::Copy(&spirv[0], spirv_code, spirv.SizeInBytes());
                    
                    free(log);
                    free(spirv_code);
                };
                auto spirv_to_msl = [](const Vector<unsigned int>& spirv, String& msl) {
                    char* msl_code = nullptr;
                    char* log = nullptr;
                    bool success = mvkConvertSPIRVToMSL((uint32_t*) spirv.Bytes(), spirv.Size(), &msl_code, &log, true, true);
                    if (!success)
                    {
                        Log("shader compile error: %s", log);
                    }
                    assert(success);
                    
                    msl = msl_code;
                    
                    free(log);
                    free(msl_code);
                };
                
                Vector<unsigned int> vs_spirv;
                Vector<unsigned int> fs_spirv;
                glsl_to_spirv(vs, filament::backend::Program::Shader::VERTEX, vs_spirv);
                glsl_to_spirv(fs, filament::backend::Program::Shader::FRAGMENT, fs_spirv);
                String vs_msl;
                String fs_msl;
                spirv_to_msl(vs_spirv, vs_msl);
                spirv_to_msl(fs_spirv, fs_msl);
                
                vs_data.Resize(vs_msl.Size());
                Memory::Copy(&vs_data[0], &vs_msl[0], vs_msl.Size());
                fs_data.Resize(fs_msl.Size());
                Memory::Copy(&fs_data[0], &fs_msl[0], fs_msl.Size());
#endif
            }
			else
			{
				vs_data.Resize(vs.Size());
				memcpy(&vs_data[0], &vs[0], vs_data.Size());
				fs_data.Resize(fs.Size());
				memcpy(&fs_data[0], &fs[0], fs_data.Size());
			}

			filament::backend::Program pb;
			pb.diagnostics(utils::CString(this->GetName().CString()))
				.withVertexShader(vs_data.Bytes(), vs_data.Size())
				.withFragmentShader(fs_data.Bytes(), fs_data.Size());

			for (int i = 0; i < pass.uniforms.Size(); ++i)
			{
				pb.setUniformBlock(pass.uniforms[i].binding, utils::CString(pass.uniforms[i].name.CString()));
			}
			
			for (int i = 0; i < pass.samplers.Size(); ++i)
			{
				const auto& group = pass.samplers[i];

				Vector<filament::backend::Program::Sampler> samplers;
				for (int j = 0; j < group.samplers.Size(); ++j)
				{
					filament::backend::Program::Sampler sampler;
					sampler.name = utils::CString(group.samplers[j].name.CString());
					sampler.binding = group.samplers[j].binding;
					samplers.Add(sampler);
				}
				pb.setSamplerGroup((size_t) group.binding, &samplers[0], samplers.Size());
			}

			pass.pipeline.program = driver.createProgram(std::move(pb));
		}
	}
}
