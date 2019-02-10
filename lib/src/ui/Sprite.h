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
#include "SpriteAtlas.h"
#include "math/Vector4.h"

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

    enum class SpriteFillMethod
    {
        Horizontal,
        Vertical,
        Radial90,
        Radial180,
        Radial360,
    };

    enum class SpriteOriginHorizontal
    {
        Left,
        Right,
    };

    enum class SpriteOriginVertical
    {
        Bottom,
        Top,
    };

    enum class SpriteOrigin90
    {
        BottomLeft,
        TopLeft,
        TopRight,
        BottomRight,
    };

    enum class SpriteOrigin180
    {
        Bottom,
        Left,
        Top,
        Right,
    };

    enum class SpriteOrigin360
    {
        Bottom,
        Left,
        Top,
        Right,
    };

    class Sprite : public View
    {
    public:
        Sprite();
        virtual ~Sprite();
        const Ref<Texture>& GetTexture() const { return m_texture; }
        void SetTexture(const Ref<Texture>& texture);
        void SetTexture(const Ref<Texture>& texture, const Recti& texture_rect, const Vector4& texture_border);
        void SetAtlas(const Ref<SpriteAtlas>& atlas);
        void SetSpriteName(const String& name);
        SpriteType GetSpriteType() const { return m_sprite_type; }
        void SetSpriteType(SpriteType type);
        void SetFillMethod(SpriteFillMethod method);
        void SetFillAmount(float amount);
        void SetFillOrigin(int origin);
        void SetFillClockWise(bool clockwise);

    protected:
        virtual void FillSelfMeshes(Vector<ViewMesh>& meshes, const Rect& clip_rect);

    private:
        Ref<Texture> m_texture;
        Recti m_texture_rect;
        Vector4 m_texture_border;
        Ref<SpriteAtlas> m_atlas;
        String m_sprite_name;
        SpriteType m_sprite_type;
        SpriteFillMethod m_fill_method;
        float m_fill_amount;
        int m_fill_origin;
        bool m_fill_clockwise;
    };
}
