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
#include "GameObject.h"
#include "graphics/Texture.h"
#include "graphics/Mesh.h"
#include "container/Map.h"
#include <functional>

namespace Viry3D
{
    class Resources
    {
    public:
		static void Init();
		static void Done();
        static Ref<GameObject> LoadGameObject(const String& path);
		static Ref<Mesh> LoadMesh(const String& path);
        static Ref<Texture> LoadTexture(const String& path);

		static void LoadFileAsync(const String& path, std::function<void(const ByteBuffer&)> complete);
		static void LoadGameObjectAsync(const String& path, std::function<void(const Ref<GameObject>&)> complete);
		static void LoadTextureAsync(const String& path, std::function<void(const Ref<Texture>&)> complete);
    };
}
