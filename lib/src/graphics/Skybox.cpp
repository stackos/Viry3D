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

#include "Skybox.h"
#include "Resources.h"

namespace Viry3D
{
	Skybox::Skybox()
    {
		auto cube = Resources::LoadMesh("Library/unity default resources.Cube.mesh");
		this->SetMesh(cube);

		auto material = RefMake<Material>(Shader::Find("Skybox"));
        material->SetVector("u_level", Vector4(0));
		this->SetMaterial(material);
    }

	Skybox::~Skybox()
    {

    }

	void Skybox::SetTexture(const Ref<Texture>& texture, float level)
	{
		this->GetMaterial()->SetTexture(MaterialProperty::TEXTURE, texture);
        this->GetMaterial()->SetVector("u_level", Vector4(level));
	}
}
