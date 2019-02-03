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
    Sprite::Sprite():
        m_rect(0, 0, 0, 0),
        m_border(0, 0, 0, 0),
        m_type(SpriteType::Simple)
    {
    
    }
    
    Sprite::~Sprite()
    {
    
    }

    void Sprite::SetTexture(const Ref<Texture>& texture)
    {
        Recti rect(0, 0, texture->GetWidth(), texture->GetHeight());
        Recti border(0, 0, texture->GetWidth(), texture->GetHeight());
        this->SetTexture(texture, rect, border);
    }

    void Sprite::SetTexture(const Ref<Texture>& texture, const Recti& rect, const Recti& border)
    {
        m_texture = texture;
        m_rect = rect;
        m_border = border;
        this->MarkCanvasDirty();
    }

    void Sprite::SetType(SpriteType type)
    {
        m_type = type;
        this->MarkCanvasDirty();
    }

    void Sprite::FillSelfMeshes(Vector<ViewMesh>& meshes, const Rect& clip_rect)
    {
        if (m_type == SpriteType::Simple)
        {
            View::FillSelfMeshes(meshes, clip_rect);

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
            else
            {
                mesh.vertices[0].uv = Vector2(m_rect.x / (float) mesh.texture->GetWidth(), m_rect.y / (float) mesh.texture->GetHeight());
                mesh.vertices[1].uv = Vector2(m_rect.x / (float) mesh.texture->GetWidth(), (m_rect.y + m_rect.h) / (float) mesh.texture->GetHeight());
                mesh.vertices[2].uv = Vector2((m_rect.x + m_rect.w) / (float) mesh.texture->GetWidth(), (m_rect.y + m_rect.h) / (float) mesh.texture->GetHeight());
                mesh.vertices[3].uv = Vector2((m_rect.x + m_rect.w) / (float) mesh.texture->GetWidth(), m_rect.y / (float) mesh.texture->GetHeight());
            }
        }
        else if (m_type == SpriteType::Sliced)
        {
            
        }
    }
}
