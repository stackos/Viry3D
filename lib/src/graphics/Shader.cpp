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

#include "Shader.h"
#include "Application.h"
#include "Texture2D.h"
#include "Debug.h"
#include "io/Directory.h"

#if VR_VULKAN
#include "vulkan/vulkan_shader_compiler.h"
#endif

namespace Viry3D
{
	Map<String, Ref<Shader>> Shader::m_shaders;
	std::mutex Shader::m_mutex;
	Map<String, Ref<Texture2D>> Shader::m_default_textures;

	static String get_shader_path(String name)
	{
		static Map<String, String> s_path_map;

		if (s_path_map.Empty())
		{
			auto dir = Application::DataPath() + "/shader";
			auto files = Directory::GetFiles(dir, true);
			for (auto& i : files)
			{
				if (i.EndsWith(".shader.xml"))
				{
					auto shader_name = i.Substring(dir.Size() + 1);
					shader_name = shader_name.Substring(0, shader_name.IndexOf("."));
					s_path_map.Add(shader_name, i);
				}
			}
		}

		String* path;
		if (s_path_map.TryGet(name, &path))
		{
			return *path;
		}

		return "";
	}

	void Shader::Init()
	{
#if VR_VULKAN
		init_compiler();
#endif
	}

	void Shader::Deinit()
	{
		m_mutex.lock();
		m_shaders.Clear();
		m_mutex.unlock();
		m_default_textures.Clear();

#if VR_VULKAN
		deinit_compiler();
#endif
	}

	void Shader::ClearAllPipelines()
	{
		for (auto& i : m_shaders)
		{
			i.second->ClearPipelines();
		}
	}

	const Ref<Texture2D>& Shader::GetDefaultTexture(const String& name)
	{
		if (!m_default_textures.Contains(name))
		{
			Ref<Texture2D> texture;

			if (name == "white")
			{
				ByteBuffer colors(4);
				int i = 0;
				colors[i++] = 255;
				colors[i++] = 255;
				colors[i++] = 255;
				colors[i++] = 255;

				texture = Texture2D::Create(1, 1, TextureFormat::RGBA32, TextureWrapMode::Clamp, FilterMode::Point, false, colors);
			}
			else if (name == "black")
			{
				ByteBuffer colors(4);
				int i = 0;
				colors[i++] = 0;
				colors[i++] = 0;
				colors[i++] = 0;
				colors[i++] = 255;

				texture = Texture2D::Create(1, 1, TextureFormat::RGBA32, TextureWrapMode::Clamp, FilterMode::Point, false, colors);
			}
			else if (name == "bump")
			{
				ByteBuffer colors(4);
				int i = 0;
				colors[i++] = 128;
				colors[i++] = 128;
				colors[i++] = 255;
				colors[i++] = 255;

				texture = Texture2D::Create(1, 1, TextureFormat::RGBA32, TextureWrapMode::Clamp, FilterMode::Point, false, colors);
			}
			else
			{
				Log("invalid default texture name!");
			}

			m_default_textures.Add(name, texture);
		}

		return m_default_textures[name];
	}

	Shader::Shader(String name)
	{
		SetName(name);
	}

	Ref<Shader> Shader::Find(String name)
	{
		Ref<Shader> shader;

		m_mutex.lock();

		Ref<Shader>* find;
		if (m_shaders.TryGet(name, &find))
		{
			shader = *find;
		}
		else
		{
			String path = get_shader_path(name);
			if (!path.Empty())
			{
				shader = Ref<Shader>(new Shader(name));
				shader->m_xml.Load(path);
				shader->Compile();

				m_shaders.Add(name, shader);
			}
			else
			{
				Log("can not find shader %s", name.CString());
			}
		}

		m_mutex.unlock();

		return shader;
	}

	int Shader::GetQueue() const
	{
		return m_xml.queue;
	}
}
