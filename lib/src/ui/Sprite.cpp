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
#include "memory/Memory.h"
#include "Debug.h"

namespace Viry3D
{
    Sprite::Sprite():
        m_texture_rect(0, 0, 0, 0),
        m_texture_border(0, 0, 0, 0),
        m_sprite_type(SpriteType::Simple),
        m_fill_method(SpriteFillMethod::Horizontal),
        m_fill_amount(1.0f),
        m_fill_origin(0),
        m_fill_clockwise(true)
    {
    
    }
    
    Sprite::~Sprite()
    {
    
    }

    void Sprite::SetTexture(const Ref<Texture>& texture)
    {
        Recti rect(0, 0, texture->GetWidth(), texture->GetHeight());
        Vector4 border(0, 0, 0, 0);
        this->SetTexture(texture, rect, border);
    }

    void Sprite::SetTexture(const Ref<Texture>& texture, const Recti& texture_rect, const Vector4& texture_border)
    {
        m_texture = texture;
        m_texture_rect = texture_rect;
        m_texture_border = texture_border;
        m_atlas.reset();
        m_sprite_name = "";
        this->MarkCanvasDirty();
    }

    void Sprite::SetAtlas(const Ref<SpriteAtlas>& atlas)
    {
        m_atlas = atlas;
        this->MarkCanvasDirty();
    }

    void Sprite::SetSpriteName(const String& name)
    {
        m_sprite_name = name;
        this->MarkCanvasDirty();
    }

    void Sprite::SetSpriteType(SpriteType type)
    {
        m_sprite_type = type;
        this->MarkCanvasDirty();
    }

    void Sprite::SetFillMethod(SpriteFillMethod method)
    {
        m_fill_method = method;
        m_fill_origin = 0;
        this->MarkCanvasDirty();
    }

    void Sprite::SetFillAmount(float amount)
    {
        m_fill_amount = Mathf::Clamp01(amount);
        this->MarkCanvasDirty();
    }

    void Sprite::SetFillOrigin(int origin)
    {
        m_fill_origin = origin;
        this->MarkCanvasDirty();
    }

    void Sprite::SetFillClockWise(bool clockwise)
    {
        m_fill_clockwise = clockwise;
        this->MarkCanvasDirty();
    }

    void Sprite::FillSelfMeshSimple(Vector<ViewMesh>& meshes, const Rect& clip_rect)
    {
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
            mesh.vertices[0].uv = Vector2(m_texture_rect.x / (float) mesh.texture->GetWidth(), m_texture_rect.y / (float) mesh.texture->GetHeight());
            mesh.vertices[1].uv = Vector2(m_texture_rect.x / (float) mesh.texture->GetWidth(), (m_texture_rect.y + m_texture_rect.h) / (float) mesh.texture->GetHeight());
            mesh.vertices[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) mesh.texture->GetWidth(), (m_texture_rect.y + m_texture_rect.h) / (float) mesh.texture->GetHeight());
            mesh.vertices[3].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) mesh.texture->GetWidth(), m_texture_rect.y / (float) mesh.texture->GetHeight());
        }
    }

    void Sprite::FillSelfMeshSliced(Vector<ViewMesh>& meshes, const Rect& clip_rect)
    {
        assert(m_texture);

        int border_l = (int) m_texture_border.x;
        int border_r = (int) m_texture_border.z;
        int border_t = (int) m_texture_border.y;
        int border_b = (int) m_texture_border.w;

        Rect rect = Rect((float) this->GetRect().x, (float) -this->GetRect().y, (float) this->GetRect().w, (float) this->GetRect().h);
        const Matrix4x4& vertex_matrix = this->GetVertexMatrix();

        Mesh::Vertex vs[16];
        Memory::Zero(&vs[0], sizeof(vs));
        int x = 0;
        int y = 0;

        vs[0].vertex = Vector3(rect.x, rect.y, 0);
        vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

        x = (int) (rect.x + border_l);
        vs[1].vertex = Vector3((float) x, vs[0].vertex.y, 0);
        vs[1].uv = Vector2((m_texture_rect.x + border_l) / (float) m_texture->GetWidth(), vs[0].uv.y);

        x = (int) (rect.x + rect.w - border_r);
        vs[2].vertex = Vector3((float) x, vs[0].vertex.y, 0);
        vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w - border_r) / (float) m_texture->GetWidth(), vs[0].uv.y);

        vs[3].vertex = Vector3(rect.x + rect.w, vs[0].vertex.y, 0);
        vs[3].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[0].uv.y);

        y = (int) (rect.y - border_t);
        vs[4].vertex = Vector3(vs[0].vertex.x, (float) y, 0);
        vs[4].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + border_t) / (float) m_texture->GetHeight());

        vs[5].vertex = Vector3(vs[1].vertex.x, vs[4].vertex.y, 0);
        vs[5].uv = Vector2(vs[1].uv.x, vs[4].uv.y);
        vs[6].vertex = Vector3(vs[2].vertex.x, vs[4].vertex.y, 0);
        vs[6].uv = Vector2(vs[2].uv.x, vs[4].uv.y);
        vs[7].vertex = Vector3(vs[3].vertex.x, vs[4].vertex.y, 0);
        vs[7].uv = Vector2(vs[3].uv.x, vs[4].uv.y);

        y = (int) (rect.y - rect.h + border_b);
        vs[8].vertex = Vector3(vs[0].vertex.x, (float) y, 0);
        vs[8].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h - border_b) / (float) m_texture->GetHeight());

        vs[9].vertex = Vector3(vs[1].vertex.x, vs[8].vertex.y, 0);
        vs[9].uv = Vector2(vs[1].uv.x, vs[8].uv.y);
        vs[10].vertex = Vector3(vs[2].vertex.x, vs[8].vertex.y, 0);
        vs[10].uv = Vector2(vs[2].uv.x, vs[8].uv.y);
        vs[11].vertex = Vector3(vs[3].vertex.x, vs[8].vertex.y, 0);
        vs[11].uv = Vector2(vs[3].uv.x, vs[8].uv.y);

        vs[12].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
        vs[12].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

        vs[13].vertex = Vector3(vs[1].vertex.x, vs[12].vertex.y, 0);
        vs[13].uv = Vector2(vs[1].uv.x, vs[12].uv.y);
        vs[14].vertex = Vector3(vs[2].vertex.x, vs[12].vertex.y, 0);
        vs[14].uv = Vector2(vs[2].uv.x, vs[12].uv.y);
        vs[15].vertex = Vector3(vs[3].vertex.x, vs[12].vertex.y, 0);
        vs[15].uv = Vector2(vs[3].uv.x, vs[12].uv.y);

        for (int i = 0; i < 16; ++i)
        {
            vs[i].vertex = vertex_matrix.MultiplyPoint3x4(vs[i].vertex);
            vs[i].color = this->GetColor();
        }

        ViewMesh mesh;
        mesh.vertices.AddRange(vs, 16);
        mesh.indices.AddRange({
            0 + 0, 4 + 0, 5 + 0, 0 + 0, 5 + 0, 1 + 0,
            0 + 1, 4 + 1, 5 + 1, 0 + 1, 5 + 1, 1 + 1,
            0 + 2, 4 + 2, 5 + 2, 0 + 2, 5 + 2, 1 + 2,

            0 + 4, 4 + 4, 5 + 4, 0 + 4, 5 + 4, 1 + 4,
            0 + 5, 4 + 5, 5 + 5, 0 + 5, 5 + 5, 1 + 5,
            0 + 6, 4 + 6, 5 + 6, 0 + 6, 5 + 6, 1 + 6,

            0 + 8, 4 + 8, 5 + 8, 0 + 8, 5 + 8, 1 + 8,
            0 + 9, 4 + 9, 5 + 9, 0 + 9, 5 + 9, 1 + 9,
            0 + 10, 4 + 10, 5 + 10, 0 + 10, 5 + 10, 1 + 10,
            });
        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshFilledHorizontal(Vector<ViewMesh>& meshes, const Rect& clip_rect, const Rect& rect, const Matrix4x4& vertex_matrix)
    {
        Mesh::Vertex vs[4];
        Memory::Zero(&vs[0], sizeof(vs));

        if (m_fill_origin == (int) SpriteOriginHorizontal::Left)
        {
            vs[0].vertex = Vector3(rect.x, rect.y, 0);
            vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

            vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
            vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

            vs[2].vertex = Vector3(rect.x + rect.w * m_fill_amount, vs[1].vertex.y, 0);
            vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w * m_fill_amount) / (float) m_texture->GetWidth(), vs[1].uv.y);

            vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
            vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);
        }
        else if (m_fill_origin == (int) SpriteOriginHorizontal::Right)
        {
            vs[0].vertex = Vector3(rect.x + rect.w * (1.0f - m_fill_amount), rect.y, 0);
            vs[0].uv = Vector2((m_texture_rect.x + m_texture_rect.w * (1.0f - m_fill_amount)) / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

            vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
            vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

            vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
            vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

            vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
            vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);
        }

        for (int i = 0; i < 4; ++i)
        {
            vs[i].vertex = vertex_matrix.MultiplyPoint3x4(vs[i].vertex);
            vs[i].color = this->GetColor();
        }

        ViewMesh mesh;
        mesh.vertices.AddRange(vs, 4);
        mesh.indices.AddRange({ 0, 1, 2, 0, 2, 3 });
        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshFilledVertical(Vector<ViewMesh>& meshes, const Rect& clip_rect, const Rect& rect, const Matrix4x4& vertex_matrix)
    {
        Mesh::Vertex vs[4];
        Memory::Zero(&vs[0], sizeof(vs));

        if (m_fill_origin == (int) SpriteOriginVertical::Bottom)
        {
            vs[0].vertex = Vector3(rect.x, rect.y - rect.h * (1.0f - m_fill_amount), 0);
            vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), (m_texture_rect.y + m_texture_rect.h * (1.0f - m_fill_amount)) / (float) m_texture->GetHeight());

            vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
            vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

            vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
            vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

            vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
            vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);
        }
        else if (m_fill_origin == (int) SpriteOriginVertical::Top)
        {
            vs[0].vertex = Vector3(rect.x, rect.y, 0);
            vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

            vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h * m_fill_amount, 0);
            vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h * m_fill_amount) / (float) m_texture->GetHeight());

            vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
            vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

            vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
            vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);
        }

        for (int i = 0; i < 4; ++i)
        {
            vs[i].vertex = vertex_matrix.MultiplyPoint3x4(vs[i].vertex);
            vs[i].color = this->GetColor();
        }

        ViewMesh mesh;
        mesh.vertices.AddRange(vs, 4);
        mesh.indices.AddRange({ 0, 1, 2, 0, 2, 3 });
        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshFilledRadial90(Vector<ViewMesh>& meshes, const Rect& clip_rect, const Rect& rect, const Matrix4x4& vertex_matrix)
    {
        Mesh::Vertex vs[5];
        Memory::Zero(&vs[0], sizeof(vs));

        vs[0].vertex = Vector3(rect.x, rect.y, 0);
        vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

        vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
        vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

        vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
        vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

        vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
        vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);

        ViewMesh mesh;

        if (m_fill_origin == (int) SpriteOrigin90::BottomLeft)
        {
            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 1, 4 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 1, 3, 1, 4, 3 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 1, 2, 4 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 1, 2, 3, 1, 3, 4 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin90::TopLeft)
        {
            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 4, 3 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 2, 3, 0, 4, 2 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 1, 4 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 0, 1, 2, 0, 2, 4 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin90::TopRight)
        {
            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 3, 4, 2 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 3, 1, 2, 3, 4, 1 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 3, 0, 4 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 3, 0, 1, 3, 1, 4 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin90::BottomRight)
        {
            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 2, 4, 1 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 2, 0, 1, 2, 4, 0 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.5f)
                {
                    float t = m_fill_amount / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 2, 3, 4 });
                }
                else
                {
                    float t = (m_fill_amount - 0.5f) / 0.5f;
                    vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[4].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 5);
                    mesh.indices.AddRange({ 2, 3, 0, 2, 0, 4 });
                }
            }
        }

        for (int i = 0; i < mesh.vertices.Size(); ++i)
        {
            mesh.vertices[i].vertex = vertex_matrix.MultiplyPoint3x4(mesh.vertices[i].vertex);
            mesh.vertices[i].color = this->GetColor();
        }

        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshFilledRadial180(Vector<ViewMesh>& meshes, const Rect& clip_rect, const Rect& rect, const Matrix4x4& vertex_matrix)
    {
        Mesh::Vertex vs[6];
        Memory::Zero(&vs[0], sizeof(vs));

        vs[0].vertex = Vector3(rect.x, rect.y, 0);
        vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

        vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
        vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

        vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
        vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

        vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
        vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);

        ViewMesh mesh;

        if (m_fill_origin == (int) SpriteOrigin180::Bottom)
        {
            vs[4].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, 0.5f);
            vs[4].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 5, 1 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 0, 1, 4, 5, 0 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 0, 1, 4, 3, 0, 4, 5, 3 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 2, 5 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 2, 3, 4, 3, 5 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 2, 3, 4, 3, 0, 4, 0, 5 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin180::Left)
        {
            vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, 0.5f);
            vs[4].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 5, 0 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 3, 0, 4, 5, 3 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 3, 0, 4, 2, 3, 4, 5, 2 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 1, 5 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 1, 2, 4, 2, 5 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 1, 2, 4, 2, 3, 4, 3, 5 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin180::Top)
        {
            vs[4].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, 0.5f);
            vs[4].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 5, 3 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 2, 3, 4, 5, 2 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 2, 3, 4, 1, 2, 4, 5, 1 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 0, 5 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 0, 1, 4, 1, 5 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 0, 1, 4, 1, 2, 4, 2, 5 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin180::Right)
        {
            vs[4].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, 0.5f);
            vs[4].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 5, 2 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 1, 2, 4, 5, 1 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 1, 2, 4, 0, 1, 4, 5, 0 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.25f)
                {
                    float t = m_fill_amount / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 3, 5 });
                }
                else if (m_fill_amount <= 0.75f)
                {
                    float t = (m_fill_amount - 0.25f) / 0.5f;
                    vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 3, 0, 4, 0, 5 });
                }
                else
                {
                    float t = (m_fill_amount - 0.75f) / 0.25f;
                    vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[5].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 6);
                    mesh.indices.AddRange({ 4, 3, 0, 4, 0, 1, 4, 1, 5 });
                }
            }
        }

        for (int i = 0; i < mesh.vertices.Size(); ++i)
        {
            mesh.vertices[i].vertex = vertex_matrix.MultiplyPoint3x4(mesh.vertices[i].vertex);
            mesh.vertices[i].color = this->GetColor();
        }

        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshFilledRadial360(Vector<ViewMesh>& meshes, const Rect& clip_rect, const Rect& rect, const Matrix4x4& vertex_matrix)
    {
        Mesh::Vertex vs[7];
        Memory::Zero(&vs[0], sizeof(vs));

        vs[0].vertex = Vector3(rect.x, rect.y, 0);
        vs[0].uv = Vector2(m_texture_rect.x / (float) m_texture->GetWidth(), m_texture_rect.y / (float) m_texture->GetHeight());

        vs[1].vertex = Vector3(vs[0].vertex.x, rect.y - rect.h, 0);
        vs[1].uv = Vector2(vs[0].uv.x, (m_texture_rect.y + m_texture_rect.h) / (float) m_texture->GetHeight());

        vs[2].vertex = Vector3(rect.x + rect.w, vs[1].vertex.y, 0);
        vs[2].uv = Vector2((m_texture_rect.x + m_texture_rect.w) / (float) m_texture->GetWidth(), vs[1].uv.y);

        vs[3].vertex = Vector3(vs[2].vertex.x, vs[0].vertex.y, 0);
        vs[3].uv = Vector2(vs[2].uv.x, vs[0].uv.y);

        vs[4].vertex = Vector3((vs[0].vertex.x + vs[3].vertex.x) / 2, (vs[0].vertex.y + vs[1].vertex.y) / 2, 0);
        vs[4].uv = Vector2((vs[0].uv.x + vs[3].uv.x) / 2, (vs[0].uv.y + vs[1].uv.y) / 2);

        ViewMesh mesh;

        if (m_fill_origin == (int) SpriteOrigin360::Bottom)
        {
            vs[5].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, 0.5f);
            vs[5].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 6, 5 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 1, 5, 4, 6, 1 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 1, 5, 4, 0, 1, 4, 6, 0 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 1, 5, 4, 0, 1, 4, 3, 0, 4, 6, 3 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 1, 5, 4, 0, 1, 4, 3, 0, 4, 2, 3, 4, 6, 2 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 6 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 2, 4, 2, 6 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 2, 4, 2, 3, 4, 3, 6 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 2, 4, 2, 3, 4, 3, 0, 4, 0, 6 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 2, 4, 2, 3, 4, 3, 0, 4, 0, 1, 4, 1, 6 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin360::Left)
        {
            vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, 0.5f);
            vs[5].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 6, 5 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 0, 5, 4, 6, 0 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 0, 5, 4, 3, 0, 4, 6, 3 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 0, 5, 4, 3, 0, 4, 2, 3, 4, 6, 2 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 0, 5, 4, 3, 0, 4, 2, 3, 4, 1, 2, 4, 6, 1 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 6 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 1, 4, 1, 6 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 1, 4, 1, 2, 4, 2, 6 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 1, 4, 1, 2, 4, 2, 3, 4, 3, 6 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4, 0, 6 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin360::Top)
        {
            vs[5].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, 0.5f);
            vs[5].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 6, 5 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 3, 5, 4, 6, 3 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 3, 5, 4, 2, 3, 4, 6, 2 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 3, 5, 4, 2, 3, 4, 1, 2, 4, 6, 1 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 3, 5, 4, 2, 3, 4, 1, 2, 4, 0, 1, 4, 6, 0 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 6 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 0, 4, 0, 6 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 0, 4, 0, 1, 4, 1, 6 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 0, 4, 0, 1, 4, 1, 2, 4, 2, 6 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 0, 4, 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 6 });
                }
            }
        }
        else if (m_fill_origin == (int) SpriteOrigin360::Right)
        {
            vs[5].vertex = Vector3::Lerp(vs[2].vertex, vs[3].vertex, 0.5f);
            vs[5].uv = Vector2::Lerp(vs[2].uv, vs[3].uv, 0.5f);

            if (m_fill_clockwise)
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 6, 5 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 2, 5, 4, 6, 2 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 2, 5, 4, 1, 2, 4, 6, 1 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 2, 5, 4, 1, 2, 4, 0, 1, 4, 6, 0 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 2, 5, 4, 1, 2, 4, 0, 1, 4, 3, 0, 4, 6, 3 });
                }
            }
            else
            {
                if (m_fill_amount <= 0.125f)
                {
                    float t = m_fill_amount / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[5].vertex, vs[3].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[5].uv, vs[3].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 6 });
                }
                else if (m_fill_amount <= 0.375f)
                {
                    float t = (m_fill_amount - 0.125f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[3].vertex, vs[0].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[3].uv, vs[0].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 3, 4, 3, 6 });
                }
                else if (m_fill_amount <= 0.625f)
                {
                    float t = (m_fill_amount - 0.375f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[0].vertex, vs[1].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[0].uv, vs[1].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 3, 4, 3, 0, 4, 0, 6 });
                }
                else if (m_fill_amount <= 0.875f)
                {
                    float t = (m_fill_amount - 0.625f) / 0.25f;
                    vs[6].vertex = Vector3::Lerp(vs[1].vertex, vs[2].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[1].uv, vs[2].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 3, 4, 3, 0, 4, 0, 1, 4, 1, 6 });
                }
                else
                {
                    float t = (m_fill_amount - 0.875f) / 0.125f;
                    vs[6].vertex = Vector3::Lerp(vs[2].vertex, vs[5].vertex, t);
                    vs[6].uv = Vector2::Lerp(vs[2].uv, vs[5].uv, t);

                    mesh.vertices.AddRange(vs, 7);
                    mesh.indices.AddRange({ 4, 5, 3, 4, 3, 0, 4, 0, 1, 4, 1, 2, 4, 2, 6 });
                }
            }
        }

        for (int i = 0; i < mesh.vertices.Size(); ++i)
        {
            mesh.vertices[i].vertex = vertex_matrix.MultiplyPoint3x4(mesh.vertices[i].vertex);
            mesh.vertices[i].color = this->GetColor();
        }

        mesh.view = this;
        mesh.base_view = false;
        mesh.clip_rect = Rect::Min(this->GetClipRect(), clip_rect);
        mesh.texture = m_texture;

        meshes.Add(mesh);
    }

    void Sprite::FillSelfMeshes(Vector<ViewMesh>& meshes, const Rect& clip_rect)
    {
        View::FillSelfMeshes(meshes, clip_rect);

        if (m_atlas && m_sprite_name.Size() > 0)
        {
            const auto& s = m_atlas->GetSprite(m_sprite_name);

            m_texture = m_atlas->GetTexture();
            m_texture_rect = s.rect;
            m_texture_border = s.border;
        }

        if (m_sprite_type == SpriteType::Simple)
        {
            this->FillSelfMeshSimple(meshes, clip_rect);
        }
        else if (m_sprite_type == SpriteType::Sliced)
        {
            this->FillSelfMeshSliced(meshes, clip_rect);
        }
        else if (m_sprite_type == SpriteType::Filled)
        {
            assert(m_texture);

            Rect rect = Rect((float) this->GetRect().x, (float) -this->GetRect().y, (float) this->GetRect().w, (float) this->GetRect().h);
            const Matrix4x4& vertex_matrix = this->GetVertexMatrix();

            if (m_fill_method == SpriteFillMethod::Horizontal)
            {
                this->FillSelfMeshFilledHorizontal(meshes, clip_rect, rect, vertex_matrix);
            }
            else if (m_fill_method == SpriteFillMethod::Vertical)
            {
                this->FillSelfMeshFilledVertical(meshes, clip_rect, rect, vertex_matrix);
            }
            else if (m_fill_method == SpriteFillMethod::Radial90)
            {
                this->FillSelfMeshFilledRadial90(meshes, clip_rect, rect, vertex_matrix);
            }
            else if (m_fill_method == SpriteFillMethod::Radial180)
            {
                this->FillSelfMeshFilledRadial180(meshes, clip_rect, rect, vertex_matrix);
            }
            else if (m_fill_method == SpriteFillMethod::Radial360)
            {
                this->FillSelfMeshFilledRadial360(meshes, clip_rect, rect, vertex_matrix);
            }
        }
    }
}
