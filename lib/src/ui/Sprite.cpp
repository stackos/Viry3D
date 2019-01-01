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

#include "Sprite.h"
#include "CanvasRenderer.h"
#include "graphics/Texture.h"

namespace Viry3D
{
    Sprite::Sprite()
    {
    
    }
    
    Sprite::~Sprite()
    {
    
    }

    void Sprite::SetTexture(const Ref<Texture>& texture)
    {
        m_texture = texture;
        this->MarkCanvasDirty();
    }

    void Sprite::FillSelfMeshes(Vector<ViewMesh>& meshes)
    {
        View::FillSelfMeshes(meshes);

        ViewMesh& mesh = meshes[meshes.Size() - 1];

        if (m_texture)
        {
            mesh.texture = m_texture;
        }
        else
        {
            mesh.texture = Texture::GetSharedWhiteTexture();
        }

        if (mesh.texture == Texture::GetSharedWhiteTexture() ||
            mesh.texture == Texture::GetSharedBlackTexture())
        {
            mesh.vertices[0].uv = Vector2(1.0f / 3, 1.0f / 3);
            mesh.vertices[1].uv = Vector2(1.0f / 3, 2.0f / 3);
            mesh.vertices[2].uv = Vector2(2.0f / 3, 2.0f / 3);
            mesh.vertices[3].uv = Vector2(2.0f / 3, 1.0f / 3);
        }
    }
}
