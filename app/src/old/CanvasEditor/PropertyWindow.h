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

#include "CanvasEditor.h"
#include "imgui/imgui.h"
#include "graphics/Camera.h"
#include "graphics/SkinnedMeshRenderer.h"
#include "ui/CanvasRenderer.h"
#include "ui/Sprite.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
	class PropertyWindow
	{
	public:
		static bool InputText(CanvasEditor* editor, const char* label, const String& buffer_name, String& target, ImGuiInputTextFlags flags = 0)
		{
			ByteBuffer& name_buffer = editor->GetTextBuffer(buffer_name);
			if (target.Size() < name_buffer.Size())
			{
				Memory::Copy(name_buffer.Bytes(), target.CString(), target.Size());
				name_buffer[target.Size()] = 0;
			}
			if (ImGui::InputText(label, (char*) name_buffer.Bytes(), name_buffer.Size(), flags))
			{
				target = String((char*) name_buffer.Bytes(), (int) strlen((char*) name_buffer.Bytes()));
				return true;
			}
			return false;
		}

		static bool LabelButton(const char* label, const char* text, const ImVec2& text_align = ImVec2(0.5f, 0.5f))
		{
			const ImVec2& button_text_align = ImGui::GetStyle().ButtonTextAlign;
			const ImVec2& item_spacing = ImGui::GetStyle().ItemSpacing;
			const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
			const ImVec2 button_size(ImGui::CalcItemWidth(), label_size.y + ImGui::GetStyle().FramePadding.y * 2);

			ImGui::GetStyle().ButtonTextAlign = text_align;
			bool pressed = ImGui::Button(text, button_size);
			ImGui::GetStyle().ButtonTextAlign = button_text_align;
			ImGui::SameLine();
			ImGui::GetStyle().ItemSpacing.x = ImGui::GetStyle().ItemInnerSpacing.x;
			ImGui::Text(label);
			ImGui::GetStyle().ItemSpacing.x = item_spacing.x;

			return pressed;
		}

		static bool DrawAlignmentCombo(const char* label, int* alignment)
		{
			bool value_changed = false;

			String alignment_str;
			switch (*alignment & 0x0000000f)
			{
			case ViewAlignment::Left:
				alignment_str += "Left";
				break;
			case ViewAlignment::HCenter:
				alignment_str += "HCenter";
				break;
			case ViewAlignment::Right:
				alignment_str += "Right";
				break;
			}
			alignment_str += "-";
			switch (*alignment & 0x000000f0)
			{
			case ViewAlignment::Top:
				alignment_str += "Top";
				break;
			case ViewAlignment::VCenter:
				alignment_str += "VCenter";
				break;
			case ViewAlignment::Bottom:
				alignment_str += "Bottom";
				break;
			}
			if (ImGui::BeginCombo(label, alignment_str.CString()))
			{
				bool select = *alignment & ViewAlignment::Left;
				if (ImGui::Selectable("Left", &select))
				{
					*alignment = (*alignment & 0xfffffff0) | ViewAlignment::Left;
					value_changed = true;
				}

				select = *alignment & ViewAlignment::HCenter;
				if (ImGui::Selectable("HCenter", &select))
				{
					*alignment = (*alignment & 0xfffffff0) | ViewAlignment::HCenter;
					value_changed = true;
				}

				select = *alignment & ViewAlignment::Right;
				if (ImGui::Selectable("Right", &select))
				{
					*alignment = (*alignment & 0xfffffff0) | ViewAlignment::Right;
					value_changed = true;
				}

				select = *alignment & ViewAlignment::Top;
				if (ImGui::Selectable("Top", &select))
				{
					*alignment = (*alignment & 0xffffff0f) | ViewAlignment::Top;
					value_changed = true;
				}

				select = *alignment & ViewAlignment::VCenter;
				if (ImGui::Selectable("VCenter", &select))
				{
					*alignment = (*alignment & 0xffffff0f) | ViewAlignment::VCenter;
					value_changed = true;
				}

				select = *alignment & ViewAlignment::Bottom;
				if (ImGui::Selectable("Bottom", &select))
				{
					*alignment = (*alignment & 0xffffff0f) | ViewAlignment::Bottom;
					value_changed = true;
				}

				ImGui::EndCombo();
			}

			return value_changed;
		}

		static void DrawObject(CanvasEditor* editor, const Ref<Object>& obj)
		{
			String name = obj->GetName();
			if (InputText(editor, "Name", "ObjectName", name))
			{
				obj->SetName(name);
			}
		}

		static void DrawNode(CanvasEditor* editor, const Ref<Node>& node)
		{
			Vector3 position = node->GetLocalPosition();
			if (ImGui::InputFloat3("Position", (float*) &position, 3))
			{
				node->SetLocalPosition(position);
			}

			Vector3 rotation = node->GetLocalRotation().ToEulerAngles();
			if (ImGui::InputFloat3("Rotation", (float*) &rotation, 3))
			{
				node->SetLocalRotation(Quaternion::Euler(rotation));
			}

			Vector3 scale = node->GetLocalScale();
			if (ImGui::InputFloat3("Scale", (float*) &scale, 3))
			{
				node->SetLocalScale(scale);
			}
		}

		static void DrawCamera(CanvasEditor* editor, const Ref<Camera>& camera)
		{
			CameraClearFlags clear_flags = camera->GetClearFlags();
			Vector<String> clear_flags_names = { "Invalidate", "Color", "Depth", "ColorAndDepth", "Nothing" };
			if (ImGui::BeginCombo("ClearFlags", clear_flags_names[(int) clear_flags].CString()))
			{
				for (int i = 0; i < clear_flags_names.Size(); ++i)
				{
					bool select = (i == (int) clear_flags);
					if (ImGui::Selectable(clear_flags_names[i].CString(), &select))
					{
						camera->SetClearFlags((CameraClearFlags) i);
					}
				}

				ImGui::EndCombo();
			}

			Color clear_color = camera->GetClearColor();
			if (ImGui::ColorEdit4("ClearColor", (float*) &clear_color))
			{
				camera->SetClearColor(clear_color);
			}

			Rect rect = camera->GetViewportRect();
			if (ImGui::InputFloat4("Rect", (float*) &rect, 3))
			{
				camera->SetViewportRect(rect);
			}

			int depth = camera->GetDepth();
			if (ImGui::InputInt("Depth", &depth))
			{
				camera->SetDepth(depth);
			}

			bool ortho = camera->IsOrthographic();
			if (ImGui::Checkbox("Orthographic", &ortho))
			{
				camera->SetOrthographic(ortho);
			}

			if (ortho)
			{
				float ortho_size = camera->GetOrthographicSize();
				if (ImGui::InputFloat("Size", &ortho_size))
				{
					camera->SetOrthographicSize(ortho_size);
				}
			}
			else
			{
				float fov = camera->GetFieldOfView();
				if (ImGui::SliderFloat("FOV", &fov, 1.0f, 179.0f))
				{
					camera->SetFieldOfView(fov);
				}
			}

			float near_clip = camera->GetNearClip();
			if (ImGui::InputFloat("Near", &near_clip))
			{
				camera->SetNearClip(near_clip);
			}

			float far_clip = camera->GetFarClip();
			if (ImGui::InputFloat("Far", &far_clip))
			{
				camera->SetFarClip(far_clip);
			}
		}

		static void DrawRenderer(CanvasEditor* editor, const Ref<Renderer>& renderer)
		{
			Vector<Ref<Material>> materials = renderer->GetMaterials();
			int material_count = materials.Size();
			if (ImGui::InputInt("MaterialCount", &material_count))
			{
				if (material_count >= 0)
				{
					materials.Resize(material_count);
					renderer->SetMaterials(materials);
				}
			}
		}

		static void DrawView(CanvasEditor* editor, const Ref<View>& view)
		{
			Color color = view->GetColor();
			if (ImGui::ColorEdit4("Color", (float*) &color))
			{
				view->SetColor(color);
			}

			int alignment = view->GetAlignment();
			if (DrawAlignmentCombo("Alignment", &alignment))
			{
				view->SetAlignment(alignment);
			}

			Vector2 pivot = view->GetPivot();
			if (ImGui::InputFloat2("Pivot", (float*) &pivot, 3))
			{
				view->SetPivot(pivot);
			}

			Vector2i size = view->GetSize();
			bool size_w_fill_parent = (size.x == VIEW_SIZE_FILL_PARENT);
			bool size_h_fill_parent = (size.y == VIEW_SIZE_FILL_PARENT);
			if (ImGui::Checkbox("WidthFillParent", &size_w_fill_parent))
			{
				if (size_w_fill_parent)
				{
					size.x = VIEW_SIZE_FILL_PARENT;
				}
				else
				{
					size.x = 100;
				}
				view->SetSize(size);
			}
			if (!size_w_fill_parent)
			{
				if (ImGui::InputInt("Width", &size.x))
				{
					view->SetSize(size);
				}
			}
			if (ImGui::Checkbox("HeightFillParent", &size_h_fill_parent))
			{
				if (size_h_fill_parent)
				{
					size.y = VIEW_SIZE_FILL_PARENT;
				}
				else
				{
					size.y = 100;
				}
				view->SetSize(size);
			}
			if (!size_h_fill_parent)
			{
				if (ImGui::InputInt("Height", &size.y))
				{
					view->SetSize(size);
				}
			}

			Vector2i offset = view->GetOffset();
			if (ImGui::InputInt2("Offset", (int*) &offset))
			{
				view->SetOffset(offset);
			}

			Vector4 margin = view->GetMargin();
			int margin_i[4] = { (int) margin.x, (int) margin.y, (int) margin.z, (int) margin.w };
			if (ImGui::InputInt4("Margin", margin_i))
			{
				margin = Vector4((float) margin_i[0], (float) margin_i[1], (float) margin_i[2], (float) margin_i[3]);
				view->SetMargin(margin);
			}

			Vector3 euler = view->GetLocalRotation().ToEulerAngles();
			if (ImGui::InputFloat3("Rotation", (float*) &euler, 3))
			{
				view->SetLocalRotation(Quaternion::Euler(euler));
			}

			Vector2 scale = view->GetLocalScale();
			if (ImGui::InputFloat2("Scale", (float*) &scale, 3))
			{
				view->SetLocalScale(scale);
			}

			bool clip_rect = view->IsClipRect();
			if (ImGui::Checkbox("ClipRect", &clip_rect))
			{
				view->EnableClipRect(clip_rect);
			}
		}

		static void DrawSprite(CanvasEditor* editor, const Ref<Sprite>& sprite)
		{
			// texture or atlas
			String texture_path;
			const char* texture_type = nullptr;

			const Ref<SpriteAtlas>& atlas = sprite->GetAtlas();
			const Ref<Texture>& texture = sprite->GetTexture();
			if (atlas)
			{
				texture_path = atlas->GetFilePath();
				texture_type = "Atlas";
			}
			else
			{
				if (texture)
				{
					texture_path = texture->GetFilePath();
				}
				texture_type = "Texture";
			}

			if (LabelButton(texture_type, texture_path.CString(), ImVec2(0, 0.5f)))
			{
				String initial_path = texture_path;
				if (initial_path.Size() == 0)
				{
					initial_path = Application::Instance()->GetDataPath();
				}
				texture_path = editor->OpenFilePanel(initial_path, "Texture or Atlas\0*.png;*.jpg;*.atlas\0");
				if (texture_path.EndsWith(".atlas"))
				{
					Ref<SpriteAtlas> atlas = SpriteAtlas::LoadFromFile(texture_path);
					if (atlas)
					{
						sprite->SetAtlas(atlas);

						Vector<String> sprite_names = atlas->GetSpriteNames();
						if (sprite_names.Size() > 0)
						{
							sprite->SetSpriteName(sprite_names[0]);
						}
					}
				}
				else if (texture_path.Size() > 0)
				{
					Ref<Texture> texture = Texture::LoadTexture2DFromFile(texture_path, FilterMode::Linear, SamplerAddressMode::ClampToEdge, false, false);
					if (texture)
					{
						sprite->SetTexture(texture);
					}
				}
			}

			if (atlas)
			{
				Vector<String> sprite_names = atlas->GetSpriteNames();
				if (ImGui::BeginCombo("Sprite", sprite->GetSpriteName().CString()))
				{
					for (int i = 0; i < sprite_names.Size(); ++i)
					{
						bool select = (sprite_names[i] == sprite->GetSpriteName());
						if (ImGui::Selectable(sprite_names[i].CString(), &select))
						{
							sprite->SetSpriteName(sprite_names[i]);
						}
					}

					ImGui::EndCombo();
				}
			}

			// sprite type
			if (atlas || texture)
			{
				SpriteType type = sprite->GetSpriteType();
				Vector<String> type_names = { "Simple", "Sliced", "Filled" };
				if (ImGui::BeginCombo("SpriteType", type_names[(int) type].CString()))
				{
					for (int i = 0; i < type_names.Size(); ++i)
					{
						bool select = (i == (int) type);
						if (ImGui::Selectable(type_names[i].CString(), &select))
						{
							sprite->SetSpriteType((SpriteType) i);
						}
					}

					ImGui::EndCombo();
				}

				if (type == SpriteType::Filled)
				{
					SpriteFillMethod fill_method = sprite->GetFillMethod();
					Vector<String> method_names = { "Horizontal", "Vertical", "Radial90", "Radial180", "Radial360" };
					if (ImGui::BeginCombo("FillMethod", method_names[(int) fill_method].CString()))
					{
						for (int i = 0; i < method_names.Size(); ++i)
						{
							bool select = (i == (int) fill_method);
							if (ImGui::Selectable(method_names[i].CString(), &select))
							{
								sprite->SetFillMethod((SpriteFillMethod) i);
							}
						}

						ImGui::EndCombo();
					}

					if (fill_method == SpriteFillMethod::Radial90 ||
						fill_method == SpriteFillMethod::Radial180 ||
						fill_method == SpriteFillMethod::Radial360)
					{
						bool fill_clockwise = sprite->IsFillClockWise();
						if (ImGui::Checkbox("FillClockWise", &fill_clockwise))
						{
							sprite->SetFillClockWise(fill_clockwise);
						}
					}

					int fill_origin = sprite->GetFillOrigin();
					Vector<String> origin_names;
					if (fill_method == SpriteFillMethod::Horizontal)
					{
						origin_names = { "Left", "Right" };
					}
					else if (fill_method == SpriteFillMethod::Vertical)
					{
						origin_names = { "Bottom", "Top" };
					}
					else if (fill_method == SpriteFillMethod::Radial90)
					{
						origin_names = { "BottomLeft", "TopLeft", "TopRight", "BottomRight" };
					}
					else if (fill_method == SpriteFillMethod::Radial180)
					{
						origin_names = { "Bottom", "Left", "Top", "Right" };
					}
					else if (fill_method == SpriteFillMethod::Radial360)
					{
						origin_names = { "Bottom", "Left", "Top", "Right" };
					}
					if (ImGui::BeginCombo("FillOrigin", origin_names[fill_origin].CString()))
					{
						for (int i = 0; i < origin_names.Size(); ++i)
						{
							bool select = (i == fill_origin);
							if (ImGui::Selectable(origin_names[i].CString(), &select))
							{
								sprite->SetFillOrigin(i);
							}
						}

						ImGui::EndCombo();
					}

					float fill_amount = sprite->GetFillAmount();
					if (ImGui::SliderFloat("FillAmount", &fill_amount, 0.0f, 1.0f))
					{
						sprite->SetFillAmount(fill_amount);
					}
				}
			}
		}

		static void DrawLabel(CanvasEditor* editor, const Ref<Label>& label)
		{
			// font
			const Ref<Font>& font = label->GetFont();
			Vector<String> font_names = { "Arial", "Consola", "PingFangSC", "SimSun" };
			String font_name;
			for (int i = 0; i < font_names.Size(); ++i)
			{
				if (font == Font::GetFont((FontType) i))
				{
					font_name = font_names[i];
					break;
				}
			}
			if (ImGui::BeginCombo("Font", font_name.CString()))
			{
				for (int i = 0; i < font_names.Size(); ++i)
				{
					bool select = (font == Font::GetFont((FontType) i));
					if (ImGui::Selectable(font_names[i].CString(), &select))
					{
						label->SetFont(Font::GetFont((FontType) i));
					}
				}

				ImGui::EndCombo();
			}

			FontStyle font_style = label->GetFontStyle();
			Vector<String> style_names = { "Normal", "Bold", "Italic", "BoldAndItalic" };
			if (ImGui::BeginCombo("FontStyle", style_names[(int) font_style].CString()))
			{
				for (int i = 0; i < style_names.Size(); ++i)
				{
					bool select = (i == (int) font_style);
					if (ImGui::Selectable(style_names[i].CString(), &select))
					{
						label->SetFontStyle((FontStyle) i);
					}
				}

				ImGui::EndCombo();
			}

			int font_size = label->GetFontSize();
			if (ImGui::InputInt("FontSize", &font_size))
			{
				if (font_size > 0)
				{
					label->SetFontSize(font_size);
				}
			}

			String text = label->GetText();
			if (InputText(editor, "Text", "LabelText", text))
			{
				label->SetText(text);
			}

			int line_space = label->GetLineSpace();
			if (ImGui::InputInt("LineSpace", &line_space))
			{
				label->SetLineSpace(line_space);
			}

			bool rich_text = label->IsRich();
			if (ImGui::Checkbox("RichText", &rich_text))
			{
				label->SetRich(rich_text);
			}

			bool mono_font = label->IsMono();
			if (ImGui::Checkbox("Mono", &mono_font))
			{
				label->SetMono(mono_font);
			}

			int text_alignment = label->GetTextAlignment();
			if (DrawAlignmentCombo("TextAlignment", &text_alignment))
			{
				label->SetTextAlignment(text_alignment);
			}

			bool wrap_content = label->IsWrapContent();
			if (ImGui::Checkbox("WrapContent", &wrap_content))
			{
				label->SetWrapContent(wrap_content);
			}
		}

		static void OnGUI(CanvasEditor* editor)
		{
			Vector<uint32_t>& selections = editor->GetSelections();

			if (selections.Size() == 1)
			{
				Ref<Object> obj = editor->GetSelectionObject(selections[0]);

				DrawObject(editor, obj);

				Ref<Node> node = RefCast<Node>(obj);
				if (node)
				{
					DrawNode(editor, node);
				}

				Ref<Camera> camera = RefCast<Camera>(obj);
				if (camera)
				{
					DrawCamera(editor, camera);
				}

				Ref<Renderer> renderer = RefCast<Renderer>(obj);
				if (renderer && !RefCast<CanvasRenderer>(obj))
				{
					DrawRenderer(editor, renderer);
				}

				Ref<View> view = RefCast<View>(obj);
				if (view)
				{
					DrawView(editor, view);
				}

				Ref<Sprite> sprite = RefCast<Sprite>(obj);
				if (sprite)
				{
					DrawSprite(editor, sprite);
				}

				Ref<Label> label = RefCast<Label>(obj);
				if (label)
				{
					DrawLabel(editor, label);
				}
			}
			else if (selections.Size() > 1)
			{
				ImGui::Text("Multi-node selected");
			}
			else
			{
				ImGui::Text("No selection");
			}
		}
	};
}
