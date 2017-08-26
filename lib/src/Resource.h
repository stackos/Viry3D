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

#pragma once

#include "Object.h"
#include "GameObject.h"
#include "graphics/Mesh.h"
#include "graphics/Texture2D.h"
#include "ui/Font.h"
#include <functional>

namespace Viry3D
{
	class ThreadPool;

	class Resource
	{
	public:
		typedef std::function<void(const Ref<Object>& obj)> LoadComplete;

		static void Init();
		static void Deinit();
		static Ref<GameObject> LoadGameObject(const String& path, bool static_batch = false, LoadComplete callback = NULL);
		static Ref<Texture> LoadTexture(const String& path);
		static Ref<Font> LoadFont(const String& path);
		static Ref<Mesh> LoadMesh(const String& path);
		static void LoadLightmapSettings(const String& path);

		static void LoadGameObjectAsync(const String& path, bool static_batch = false, LoadComplete callback = NULL);
		static void LoadTextureAsync(const String& path, LoadComplete callback = NULL);
		static void LoadFontAsync(const String& path, LoadComplete callback = NULL);
		static void LoadMeshAsync(const String& path, LoadComplete callback = NULL);

	private:
		static Ref<ThreadPool> m_thread_res_load;
	};
}
