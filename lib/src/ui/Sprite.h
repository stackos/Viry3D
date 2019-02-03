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

#include "View.h"

namespace Viry3D
{
    class Texture;

    enum class SpriteType
    {
        Simple,
        Sliced,
        Tiled,
        Filled,
    };

    class Sprite : public View
    {
    public:
        Sprite();
        virtual ~Sprite();
        const Ref<Texture>& GetTexture() const { return m_texture; }
        void SetTexture(const Ref<Texture>& texture);
        void SetTexture(const Ref<Texture>& texture, const Recti& rect, const Recti& border);
        SpriteType GetType() const { return m_type; }
        void SetType(SpriteType type);
    
    protected:
        virtual void FillSelfMeshes(Vector<ViewMesh>& meshes, const Rect& clip_rect);

    private:
        Ref<Texture> m_texture;
        Recti m_rect;
        Recti m_border;
        SpriteType m_type;
    };
}
