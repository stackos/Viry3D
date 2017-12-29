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

#include "UISprite.h"
#include "graphics/Material.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(UISprite);

	UISprite::UISprite():
		m_sprite_type(SpriteType::Simple),
		m_fill_method(SpriteFillMethod::Horizontal),
		m_fill_origin((int) SpriteFillOriginHorizontal::Left),
		m_fill_amount(1),
		m_fill_clock_wise(false)
	{
	}

	void UISprite::DeepCopy(const Ref<Object>& source)
	{
		UIView::DeepCopy(source);

		auto src = RefCast<UISprite>(source);
		m_atlas = src->m_atlas;
		m_sprite_name = src->m_sprite_name;
		m_sprite_type = src->m_sprite_type;
		m_fill_method = src->m_fill_method;
		m_fill_origin = src->m_fill_origin;
		m_fill_amount = src->m_fill_amount;
		m_fill_clock_wise = src->m_fill_clock_wise;
	}

	void UISprite::SetAtlas(const Ref<Atlas>& atlas)
	{
		if (m_atlas != atlas)
		{
			m_atlas = atlas;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::SetSpriteName(const String& name)
	{
		if (m_sprite_name != name)
		{
			m_sprite_name = name;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

    void UISprite::SetSingleTexture(const Ref<Texture2D>& texture)
    {
        m_single_texture = texture;
    }

	void UISprite::SetSpriteType(SpriteType type)
	{
		if (m_sprite_type != type)
		{
			m_sprite_type = type;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::SetFillMethod(SpriteFillMethod fill_method)
	{
		if (m_fill_method != fill_method)
		{
			m_fill_method = fill_method;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::SetFillOrigin(int fill_origin)
	{
		if (m_fill_origin != fill_origin)
		{
			m_fill_origin = fill_origin;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::SetFillAmount(float fill_amount)
	{
		if (m_fill_amount != fill_amount)
		{
			m_fill_amount = fill_amount;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::SetFillClockWise(bool fill_clock_wise)
	{
		if (m_fill_clock_wise != fill_clock_wise)
		{
			m_fill_clock_wise = fill_clock_wise;
			m_dirty = true;
			MarkRendererDirty();
		}
	}

	void UISprite::FillVerticesSimple(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices)
	{
		GetBoundsVertices(vertices);

		Vector2 uv_array[4];
		if (m_atlas)
		{
			auto sprite = m_atlas->GetSprite(m_sprite_name);
			auto rect = sprite->GetRect();

			uv_array[0] = Vector2(rect.x, 1 - rect.y);
			uv_array[1] = Vector2(rect.x + rect.width, 1 - rect.y);
			uv_array[2] = Vector2(rect.x + rect.width, 1 - (rect.y + rect.height));
			uv_array[3] = Vector2(rect.x, 1 - (rect.y + rect.height));
		}
		else
		{
			uv_array[0] = Vector2(0, 1);
			uv_array[1] = Vector2(1, 1);
			uv_array[2] = Vector2(1, 0);
			uv_array[3] = Vector2(0, 0);
		}

		uv.AddRange(uv_array, 4);

		colors.Add(m_color);
		colors.Add(m_color);
		colors.Add(m_color);
		colors.Add(m_color);

		int index_begin = vertices.Size() - 4;
		indices.Add(index_begin + 0);
		indices.Add(index_begin + 1);
		indices.Add(index_begin + 2);
		indices.Add(index_begin + 0);
		indices.Add(index_begin + 2);
		indices.Add(index_begin + 3);
	}

	void UISprite::FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices)
	{
		switch (m_sprite_type)
		{
			case SpriteType::Simple:
				this->FillVerticesSimple(vertices, uv, colors, indices);
				break;
			default:
				break;
		}
	}

	void UISprite::FillMaterial(Ref<Material>& mat)
	{
		if (m_atlas)
		{
			mat->SetMainTexture(m_atlas->GetTexture());
		}
        else if(m_single_texture)
        {
            mat->SetMainTexture(m_single_texture);
        }
	}
}
