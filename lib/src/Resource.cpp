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

#include "Resource.h"
#include "Application.h"
#include "GameObject.h"
#include "World.h"
#include "Debug.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "graphics/Display.h"
#include "graphics/LightmapSettings.h"
#include "graphics/Cubemap.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UISprite.h"
#include "ui/Atlas.h"
#include "ui/UILabel.h"
#include "renderer/MeshRenderer.h"
#include "renderer/SkinnedMeshRenderer.h"
#include "renderer/ParticleSystemRenderer.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Terrain.h"
#include "thread/Thread.h"
#include "animation/AnimationClip.h"
#include "animation/Animation.h"

namespace Viry3D
{
	Ref<ThreadPool> Resource::m_thread_res_load;

	static String read_string(MemoryStream& ms)
	{
		auto size = ms.Read<int>();
		return ms.ReadString(size);
	}

	static Ref<Texture> read_texture(const String& path)
	{
		Ref<Texture> texture;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return texture;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			texture = RefCast<Texture>(cache);
		}
		else
		{
			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			auto texture_name = read_string(ms);
			auto width = ms.Read<int>();
			auto height = ms.Read<int>();
			auto wrap_mode = (TextureWrapMode) ms.Read<int>();
			auto filter_mode = (FilterMode) ms.Read<int>();
			auto texture_type = read_string(ms);

			if (texture_type == "Texture2D")
			{
				int mipmap_count = ms.Read<int>();
				auto png_path = read_string(ms);
				png_path = Application::DataPath() + png_path.Substring(String("Assets").Size());

				texture = Texture2D::LoadFromFile(png_path, wrap_mode, filter_mode, mipmap_count > 1);

				if (texture)
				{
					Object::AddCache(path, texture);
				}
			}
			else if (texture_type == "Texture2DRGBFloat")
			{
				int mipmap_count = ms.Read<int>();
				auto data_path = read_string(ms);
				data_path = Application::DataPath() + data_path.Substring(String("Assets").Size());
				auto colors = File::ReadAllBytes(data_path);

				texture = Texture2D::Create(width, height, TextureFormat::RGBFloat, wrap_mode, filter_mode, mipmap_count > 1, colors);
				Object::AddCache(path, texture);
			}
			else if (texture_type == "Cubemap")
			{
				int mipmap_count = ms.Read<int>();

				auto cubemap = Cubemap::Create(width, TextureFormat::RGBA32, wrap_mode, filter_mode, mipmap_count > 1);

				for (int i = 0; i < mipmap_count; i++)
				{
					for (int j = 0; j < 6; j++)
					{
						auto face_path = read_string(ms);
						face_path = Application::DataPath() + face_path.Substring(String("Assets").Size());

						ByteBuffer colors;
						int w, h;
						TextureFormat format;
						auto buffer = File::ReadAllBytes(face_path);
						if (Texture2D::LoadImageData(buffer, colors, w, h, format))
						{
							cubemap->SetPixels(colors, (CubemapFace) j, i);
						}
					}
				}

				cubemap->Apply(false, true);

				texture = cubemap;
				Object::AddCache(path, texture);
			}
			else if(texture_type == "CubemapRGBFloat")
			{
				int mipmap_count = ms.Read<int>();

				auto cubemap = Cubemap::Create(width, TextureFormat::RGBFloat, wrap_mode, filter_mode, mipmap_count > 1);
				
				for (int i = 0; i < mipmap_count; i++)
				{
					for (int j = 0; j < 6; j++)
					{
						auto face_path = read_string(ms);
						face_path = Application::DataPath() + face_path.Substring(String("Assets").Size());
						auto colors = File::ReadAllBytes(face_path);
						cubemap->SetPixels(colors, (CubemapFace) j, i);
					}
				}

				cubemap->Apply(false, true);

				texture = cubemap;
				Object::AddCache(path, texture);
			}

			ms.Close();
		}

		return texture;
	}

	static Ref<Atlas> read_atlas(const String& path)
	{
		Ref<Atlas> atlas;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return atlas;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			atlas = RefCast<Atlas>(cache);
		}
		else
		{
			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			atlas = Atlas::Create();

			auto texture_path = read_string(ms);
			auto texture = RefCast<Texture2D>(read_texture(texture_path));
			atlas->SetTexture(texture);

			auto sprite_count = ms.Read<int>();
			for (int i = 0; i < sprite_count; i++)
			{
				auto name = read_string(ms);
				auto rect = ms.Read<Rect>();
				ms.Read<Vector2>();//pivot
				ms.Read<float>();//pixel per unit
				auto border = ms.Read<Vector4>();

				auto sprite = Sprite::Create(rect, border);
				sprite->SetName(name);
				atlas->AddSprite(name, sprite);
			}

			Object::AddCache(path, atlas);

			ms.Close();
		}

		return atlas;
	}

	static void read_image(MemoryStream& ms, Ref<UISprite>& view)
	{
		auto color = ms.Read<Color>();
		auto sprite_type = (SpriteType) ms.Read<int>();
		auto fill_method = (SpriteFillMethod) ms.Read<int>();
		auto fill_origin = ms.Read<int>();
		auto fill_amount = ms.Read<float>();
		auto fill_clock_wise = ms.Read<bool>();
		auto sprite_name = read_string(ms);

		if (!sprite_name.Empty())
		{
			auto atlas_path = read_string(ms);
			auto atlas = read_atlas(atlas_path);

			view->SetAtlas(atlas);
			view->SetSpriteName(sprite_name);
		}

		view->SetColor(color);
		view->SetSpriteType(sprite_type);
		view->SetFillMethod(fill_method);
		view->SetFillOrigin(fill_origin);
		view->SetFillAmount(fill_amount);
		view->SetFillClockWise(fill_clock_wise);
	}

	static Ref<Font> read_font(const String& path)
	{
		Ref<Font> font;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return font;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			font = RefCast<Font>(cache);
		}
		else
		{
			font = Font::LoadFromFile(full_path);

			if (font)
			{
				Object::AddCache(path, font);
			}
		}

		return font;
	}

	static void read_label(MemoryStream& ms, Ref<UILabel>& view)
	{
		auto color = ms.Read<Color>();
		auto text = read_string(ms);
		auto font_name = read_string(ms);
		auto font_style = read_string(ms);
		auto font_size = ms.Read<int>();
		auto line_space = ms.Read<float>();
		auto rich = ms.Read<bool>();
		auto alignment = read_string(ms);

		if (!font_name.Empty())
		{
			auto font = read_font("Assets/font/" + font_name + ".ttf");

			view->SetFont(font);

			if (font_style == "Normal")
			{
				view->SetFontStyle(FontStyle::Normal);
			}
			else if (font_style == "Bold")
			{
				view->SetFontStyle(FontStyle::Bold);
			}
			else if (font_style == "Italic")
			{
				view->SetFontStyle(FontStyle::Italic);
			}
			else if (font_style == "BoldAndItalic")
			{
				view->SetFontStyle(FontStyle::BoldAndItalic);
			}

			view->SetFontSize(font_size);
		}

		view->SetColor(color);
		view->SetText(text);
		view->SetLineSpace((int) line_space);
		view->SetRich(rich);

		if (alignment == "UpperLeft")
		{
			view->SetAlignment(TextAlignment::UpperLeft);
		}
		else if (alignment == "UpperCenter")
		{
			view->SetAlignment(TextAlignment::UpperCenter);
		}
		else if (alignment == "UpperRight")
		{
			view->SetAlignment(TextAlignment::UpperRight);
		}
		else if (alignment == "MiddleLeft")
		{
			view->SetAlignment(TextAlignment::MiddleLeft);
		}
		else if (alignment == "MiddleCenter")
		{
			view->SetAlignment(TextAlignment::MiddleCenter);
		}
		else if (alignment == "MiddleRight")
		{
			view->SetAlignment(TextAlignment::MiddleRight);
		}
		else if (alignment == "LowerLeft")
		{
			view->SetAlignment(TextAlignment::LowerLeft);
		}
		else if (alignment == "LowerCenter")
		{
			view->SetAlignment(TextAlignment::LowerCenter);
		}
		else if (alignment == "LowerRight")
		{
			view->SetAlignment(TextAlignment::LowerRight);
		}
	}

	static void read_rect(MemoryStream& ms, Ref<UIRect>& rect)
	{
		auto anchor_min = ms.Read<Vector2>();
		auto anchor_max = ms.Read<Vector2>();
		auto offset_min = ms.Read<Vector2>();
		auto offset_max = ms.Read<Vector2>();
		auto pivot = ms.Read<Vector2>();

		rect->SetAnchors(anchor_min, anchor_max);
		rect->SetOffsets(offset_min, offset_max);
		rect->SetPivot(pivot);

		auto canvas = RefCast<UICanvasRenderer>(rect);
		if (canvas && canvas->IsRootCanvas())
		{
			int screen_w = Graphics::GetDisplay()->GetWidth();
			int screen_h = Graphics::GetDisplay()->GetHeight();
			canvas->SetSize(Vector2((float) screen_w, (float) screen_h));
		}

		rect->OnAnchor();
	}

	static void read_canvas(MemoryStream& ms, Ref<UICanvasRenderer>& canvas)
	{
		auto sorting_order = ms.Read<int>();

		canvas->SetSortingOrder(sorting_order);
	}

	static Ref<Mesh> read_mesh(const String& path)
	{
		Ref<Mesh> mesh;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return mesh;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			mesh = RefCast<Mesh>(cache);
		}
		else
		{
			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			mesh = Mesh::Create();
			Object::AddCache(path, mesh);

			auto mesh_name = read_string(ms);
			mesh->SetName(mesh_name);

			auto vertex_count = ms.Read<int>();
			if (vertex_count > 0)
			{
				mesh->vertices.Resize(vertex_count);
				ms.Read(&mesh->vertices[0], vertex_count * sizeof(Vector3));
			}

			auto uv_count = ms.Read<int>();
			if (uv_count > 0)
			{
				mesh->uv.Resize(uv_count);
				ms.Read(&mesh->uv[0], uv_count * sizeof(Vector2));
			}

			auto color_count = ms.Read<int>();
			if (color_count > 0)
			{
				mesh->colors.Resize(color_count);
				ms.Read(&mesh->colors[0], color_count * sizeof(Color));
			}

			auto uv2_count = ms.Read<int>();
			if (uv2_count > 0)
			{
				mesh->uv2.Resize(uv2_count);
				ms.Read(&mesh->uv2[0], uv2_count * sizeof(Vector2));
			}

			auto normal_count = ms.Read<int>();
			if (normal_count > 0)
			{
				mesh->normals.Resize(normal_count);
				ms.Read(&mesh->normals[0], normal_count * sizeof(Vector3));
			}

			auto tangent_count = ms.Read<int>();
			if (tangent_count > 0)
			{
				mesh->tangents.Resize(tangent_count);
				ms.Read(&mesh->tangents[0], tangent_count * sizeof(Vector4));
			}

			auto bone_weight_count = ms.Read<int>();
			if (bone_weight_count > 0)
			{
				mesh->bone_weights.Resize(bone_weight_count);
				ms.Read(&mesh->bone_weights[0], bone_weight_count * sizeof(Vector4));
			}

			auto bone_index_count = ms.Read<int>();
			if (bone_index_count > 0)
			{
				mesh->bone_indices.Resize(bone_index_count);
				ms.Read(&mesh->bone_indices[0], bone_index_count * sizeof(Vector4));
			}

			auto bind_pose_count = ms.Read<int>();
			if (bind_pose_count > 0)
			{
				mesh->bind_poses.Resize(bind_pose_count);
				ms.Read(&mesh->bind_poses[0], bind_pose_count * sizeof(Matrix4x4));
			}

			auto index_count = ms.Read<int>();
			if (index_count > 0)
			{
				mesh->triangles.Resize(index_count);
				ms.Read(&mesh->triangles[0], index_count * sizeof(unsigned short));
			}

			auto submesh_count = ms.Read<int>();
			if (submesh_count > 0)
			{
				mesh->submeshes.Resize(submesh_count);
				ms.Read(&mesh->submeshes[0], submesh_count * sizeof(Mesh::Submesh));
			}

			auto blend_shape_count = ms.Read<int>();
			if (blend_shape_count > 0)
			{
				mesh->blend_shapes.Resize(blend_shape_count);

				for (int i = 0; i < blend_shape_count; i++)
				{
					mesh->blend_shapes[i].name = read_string(ms);
					auto frame_count = ms.Read<int>();
					if (frame_count > 0)
					{
						mesh->blend_shapes[i].frames.Resize(frame_count);

						for (int j = 0; j < frame_count; j++)
						{
							mesh->blend_shapes[i].frames[j].weight = ms.Read<float>();
							mesh->blend_shapes[i].frames[j].deltas.Resize(vertex_count);

							ms.Read(&mesh->blend_shapes[i].frames[j].deltas[0], sizeof(Mesh::BlendShapeVertexDelta) * vertex_count);
						}
					}
				}

				mesh->SetDynamic(true);
			}

			ms.Close();

			mesh->Apply();
		}

		return mesh;
	}

	static Ref<Material> read_material(const String& path)
	{
		Ref<Material> mat;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return mat;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			mat = RefCast<Material>(cache);
		}
		else
		{
			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			auto mat_name = read_string(ms);
			auto shader_name = read_string(ms);

			mat = Material::Create(shader_name);
			Object::AddCache(path, mat);

			mat->SetName(mat_name);

			auto property_count = ms.Read<int>();
			for (int i = 0; i < property_count; i++)
			{
				auto property_name = read_string(ms);
				auto property_type = read_string(ms);

				if (property_type == "Color")
				{
					auto c = ms.Read<Color>();
					mat->SetColor(property_name, c);
				}
				else if (property_type == "Vector")
				{
					auto v = ms.Read<Vector4>();
					mat->SetVector(property_name, v);
				}
				else if (property_type == "Float" || property_type == "Range")
				{
					auto f = ms.Read<float>();
					mat->SetVector(property_name, Vector4(f, 0, 0, 0));
				}
				else if (property_type == "TexEnv")
				{
					auto tex_st = ms.Read<Vector4>();
					auto tex_path = read_string(ms);

					if (!tex_path.Empty())
					{
						auto texture = read_texture(tex_path);

						if (texture)
						{
							mat->SetTexture(property_name, texture);
							mat->SetVector(property_name + "_ST", tex_st);
						}
					}
				}
			}

			ms.Close();
		}

		return mat;
	}

	static void read_renderer_materials(MemoryStream& ms, Renderer* renderer)
	{
		auto mat_count = ms.Read<int>();
		if (mat_count > 0)
		{
			Vector<Ref<Material>> mats(mat_count);

			for (int i = 0; i < mat_count; i++)
			{
				auto mat_path = read_string(ms);

				if (!mat_path.Empty())
				{
					auto mat = read_material(mat_path);

					mats[i] = mat;
				}
			}

			renderer->SetSharedMaterials(mats);
		}
	}

	static void read_mesh_renderer(MemoryStream& ms, Ref<MeshRenderer>& renderer)
	{
		auto mesh_path = read_string(ms);
		if (!mesh_path.Empty())
		{
			auto mesh = read_mesh(mesh_path);
			renderer->SetSharedMesh(mesh);
		}

		auto bounds_min = ms.Read<Vector3>();
		auto bounds_max = ms.Read<Vector3>();
		renderer->SetBounds(Bounds(bounds_min, bounds_max));

		read_renderer_materials(ms, renderer.get());

		auto lightmap_index = ms.Read<int>();
		auto lightmap_scale_offset = ms.Read<Vector4>();

		renderer->SetLightmapIndex(lightmap_index);
		renderer->SetLightmapScaleOffset(lightmap_scale_offset);
	}

	static void read_skinned_mesh_renderer(
		MemoryStream& ms,
		Ref<SkinnedMeshRenderer>& renderer,
		Map<int, Ref<Transform>>& transform_instances)
	{
		auto mesh_path = read_string(ms);
		if (!mesh_path.Empty())
		{
			auto mesh = read_mesh(mesh_path);
			renderer->SetSharedMesh(mesh);
		}

		auto bounds_min = ms.Read<Vector3>();
		auto bounds_max = ms.Read<Vector3>();
		renderer->SetBounds(Bounds(bounds_min, bounds_max));

		read_renderer_materials(ms, renderer.get());

		auto bone_count = ms.Read<int>();
		if (bone_count > 0)
		{
			Vector<WeakRef<Transform>> bones(bone_count);

			for (int i = 0; i < bone_count; i++)
			{
				int bone_id = ms.Read<int>();

				bones[i] = transform_instances[bone_id];
			}

			renderer->SetBones(bones);

			if (bone_count > BONE_MAX)
			{
				renderer->Enable(false);
				Log("bone count %d over than %d", bone_count, BONE_MAX);
			}
		}
	}

	static void read_animation_curve(MemoryStream& ms, AnimationCurve* curve)
	{
		ms.Read<int>();	//	pre_wrap_mode
		ms.Read<int>();	//	post_wrap_mode
		auto frame_count = ms.Read<int>();

		for (int j = 0; j < frame_count; j++)
		{
			auto time = ms.Read<float>();
			auto value = ms.Read<float>();
			auto in_tangent = ms.Read<float>();
			auto out_tangent = ms.Read<float>();
			ms.Read<int>();	//	tangent_mode

			if (curve)
			{
				curve->keys.Add(Keyframe(time, value, in_tangent, out_tangent));
			}
		}
	}

	static Ref<AnimationClip> read_animation_clip(const String& path)
	{
		Ref<AnimationClip> clip;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return clip;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			clip = RefCast<AnimationClip>(cache);
		}
		else
		{
			clip = RefMake<AnimationClip>();
			Object::AddCache(path, clip);

			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			auto name = read_string(ms);
			clip->SetName(name);
			clip->frame_rate = ms.Read<float>();
			clip->length = ms.Read<float>();
			clip->wrap_mode = (AnimationWrapMode) ms.Read<int>();
			auto curve_count = ms.Read<int>();

			for (int i = 0; i < curve_count; i++)
			{
				auto path = read_string(ms);
				auto property = read_string(ms);

				int property_index = -1;

				const String property_names[] = {
					"m_LocalPosition.x",
					"m_LocalPosition.y",
					"m_LocalPosition.z",
					"m_LocalRotation.x",
					"m_LocalRotation.y",
					"m_LocalRotation.z",
					"m_LocalRotation.w",
					"m_LocalScale.x",
					"m_LocalScale.y",
					"m_LocalScale.z",
				};
				for (int j = 0; j < (int) CurveProperty::Count; j++)
				{
					if (property == property_names[j])
					{
						property_index = j;
						break;
					}
				}

				AnimationCurve* curve = NULL;
				if (property_index >= 0)
				{
					CurveBinding* p_binding;
					if (!clip->curves.TryGet(path, &p_binding))
					{
						clip->curves.Add(path, CurveBinding());
						p_binding = &clip->curves[path];
						p_binding->path = path;
						p_binding->transform_curves.Resize((int) CurveProperty::Count);
					}

					curve = &p_binding->transform_curves[property_index];
				}
				else
				{
					if (property.StartsWith("blendShape"))
					{
						CurveBinding* p_binding;
						if (!clip->curves.TryGet(path, &p_binding))
						{
							clip->curves.Add(path, CurveBinding());
							p_binding = &clip->curves[path];
							p_binding->path = path;
						}

						p_binding->blend_shape_properties.Add(property);
						p_binding->blend_shape_curves.Add(AnimationCurve());
						curve = &p_binding->blend_shape_curves[p_binding->blend_shape_curves.Size() - 1];
					}
				}

				read_animation_curve(ms, curve);
			}

			ms.Close();
		}

		return clip;
	}

	static void read_animation(MemoryStream& ms, Ref<Animation>& animation)
	{
		auto defaul_clip = read_string(ms);
		auto clip_count = ms.Read<int>();

		Map<String, AnimationState> states;

		for (int i = 0; i < clip_count; i++)
		{
			auto clip_path = read_string(ms);

			if (!clip_path.Empty())
			{
				auto clip = read_animation_clip(clip_path);

				if (clip)
				{
					states.Add(clip->GetName(), AnimationState(clip));
				}
			}
		}

		animation->SetAnimationStates(states);
	}

	static void read_min_max_curve(MemoryStream& ms, ParticleSystem::MinMaxCurve& curve)
	{
		curve.mode = (ParticleSystemCurveMode) ms.Read<int>();
		if (curve.mode == ParticleSystemCurveMode::Constant)
		{
			curve.constant = ms.Read<float>();
		}
		else if (curve.mode == ParticleSystemCurveMode::Curve)
		{
			read_animation_curve(ms, &curve.curve);
		}
		else if (curve.mode == ParticleSystemCurveMode::TwoCurves)
		{
			read_animation_curve(ms, &curve.curve_min);
			read_animation_curve(ms, &curve.curve_max);
		}
		else if (curve.mode == ParticleSystemCurveMode::TwoConstants)
		{
			curve.constant_min = ms.Read<float>();
			curve.constant_max = ms.Read<float>();
		}
	}

	static void read_gradient(MemoryStream& ms, Gradient& grad)
	{
		grad.mode = (GradientMode) ms.Read<int>();

		int color_len = ms.Read<int>();
		for (int i = 0; i < color_len; i++)
		{
			float time = ms.Read<float>();
			Color rgb = ms.Read<Color>();

			grad.r.keys.Add(Keyframe(time, rgb.r));
			grad.g.keys.Add(Keyframe(time, rgb.g));
			grad.b.keys.Add(Keyframe(time, rgb.b));
		}

		int alpha_len = ms.Read<int>();
		for (int i = 0; i < alpha_len; i++)
		{
			float time = ms.Read<float>();
			float alpha = ms.Read<float>();

			grad.a.keys.Add(Keyframe(time, alpha));
		}
	}

	static void read_min_max_gradient(MemoryStream& ms, ParticleSystem::MinMaxGradient& grad)
	{
		grad.mode = (ParticleSystemGradientMode) ms.Read<int>();
		if (grad.mode == ParticleSystemGradientMode::Color)
		{
			grad.color = ms.Read<Color>();
		}
		else if (grad.mode == ParticleSystemGradientMode::Gradient)
		{
			read_gradient(ms, grad.gradient);
		}
		else if (grad.mode == ParticleSystemGradientMode::TwoColors)
		{
			grad.color_min = ms.Read<Color>();
			grad.color_max = ms.Read<Color>();
		}
		else if (grad.mode == ParticleSystemGradientMode::TwoGradients)
		{
			read_gradient(ms, grad.gradient_min);
			read_gradient(ms, grad.gradient_max);
		}
		else if (grad.mode == ParticleSystemGradientMode::RandomColor)
		{
			read_gradient(ms, grad.gradient);
		}
	}

	static void read_particle_main(MemoryStream& ms, ParticleSystem::MainModule& module)
	{
		module.duration = ms.Read<float>();
		module.loop = ms.Read<bool>();

		read_min_max_curve(ms, module.start_delay);
		read_min_max_curve(ms, module.start_lifetime);
		read_min_max_curve(ms, module.start_speed);

		module.start_size_3d = ms.Read<bool>();
		if (module.start_size_3d)
		{
			read_min_max_curve(ms, module.start_size_x);
			read_min_max_curve(ms, module.start_size_y);
			read_min_max_curve(ms, module.start_size_z);
		}
		else
		{
			read_min_max_curve(ms, module.start_size);
		}

		module.start_rotation_3d = ms.Read<bool>();
		if (module.start_rotation_3d)
		{
			read_min_max_curve(ms, module.start_rotation_x);
			read_min_max_curve(ms, module.start_rotation_y);
			read_min_max_curve(ms, module.start_rotation_z);
		}
		else
		{
			read_min_max_curve(ms, module.start_rotation);
		}

		module.randomize_rotation_direction = ms.Read<float>();

		read_min_max_gradient(ms, module.start_color);
		read_min_max_curve(ms, module.gravity_modifier);
		module.simulation_space = (ParticleSystemSimulationSpace) ms.Read<int>();
		module.simulation_speed = ms.Read<float>();
		module.scaling_mode = (ParticleSystemScalingMode) ms.Read<int>();
		module.max_particles = ms.Read<int>();
	}

	static void read_particle_emission(MemoryStream& ms, ParticleSystem::EmissionModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		read_min_max_curve(ms, module.rate_over_time);
		read_min_max_curve(ms, module.rate_over_distance);

		auto burst_count = ms.Read<int>();
		module.bursts.Resize(burst_count);
		for (int i = 0; i < burst_count; i++)
		{
			module.bursts[i].time = ms.Read<float>();
			module.bursts[i].min_count = ms.Read<short>();
			module.bursts[i].max_count = ms.Read<short>();
			module.bursts[i].cycle_count = ms.Read<int>();
			module.bursts[i].repeat_interval = ms.Read<float>();
		}
	}

	static void read_particle_shape(MemoryStream& ms, ParticleSystem::ShapeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.shape_type = (ParticleSystemShapeType) ms.Read<int>();
		if (module.shape_type == ParticleSystemShapeType::Sphere ||
			module.shape_type == ParticleSystemShapeType::SphereShell ||
			module.shape_type == ParticleSystemShapeType::Hemisphere ||
			module.shape_type == ParticleSystemShapeType::HemisphereShell)
		{
			module.radius = ms.Read<float>();
		}
		else if (module.shape_type == ParticleSystemShapeType::Cone ||
			module.shape_type == ParticleSystemShapeType::ConeShell ||
			module.shape_type == ParticleSystemShapeType::ConeVolume ||
			module.shape_type == ParticleSystemShapeType::ConeVolumeShell)
		{
			module.angle = ms.Read<float>();
			module.radius = ms.Read<float>();
			module.arc = ms.Read<float>();
			module.arc_mode = (ParticleSystemShapeMultiModeValue) ms.Read<int>();
			module.arc_spread = ms.Read<float>();
			read_min_max_curve(ms, module.arc_speed);
			module.length = ms.Read<float>();
		}
		else if (module.shape_type == ParticleSystemShapeType::Box ||
			module.shape_type == ParticleSystemShapeType::BoxShell ||
			module.shape_type == ParticleSystemShapeType::BoxEdge)
		{
			module.box = ms.Read<Vector3>();
		}
		else if (module.shape_type == ParticleSystemShapeType::Circle ||
			module.shape_type == ParticleSystemShapeType::CircleEdge)
		{
			module.radius = ms.Read<float>();
			module.arc = ms.Read<float>();
			module.arc_mode = (ParticleSystemShapeMultiModeValue) ms.Read<int>();
			module.arc_spread = ms.Read<float>();
			read_min_max_curve(ms, module.arc_speed);
		}
		else if (module.shape_type == ParticleSystemShapeType::SingleSidedEdge)
		{
			module.radius = ms.Read<float>();
			module.radius_mode = (ParticleSystemShapeMultiModeValue) ms.Read<int>();
			module.radius_spread = ms.Read<float>();
			read_min_max_curve(ms, module.radius_speed);
		}

		module.align_to_direction = ms.Read<bool>();
		module.random_direction_amount = ms.Read<float>();
		module.spherical_direction_amount = ms.Read<float>();
	}

	static void read_particle_velocity_over_lifetime(MemoryStream& ms, ParticleSystem::VelocityOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		read_min_max_curve(ms, module.x);
		read_min_max_curve(ms, module.y);
		read_min_max_curve(ms, module.z);
		module.space = (ParticleSystemSimulationSpace) ms.Read<int>();
	}

	static void read_particle_limit_velocity_over_lifetime(MemoryStream& ms, ParticleSystem::LimitVelocityOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.separate_axes = ms.Read<bool>();
		if (module.separate_axes)
		{
			read_min_max_curve(ms, module.limit_x);
			read_min_max_curve(ms, module.limit_y);
			read_min_max_curve(ms, module.limit_z);
			module.space = (ParticleSystemSimulationSpace) ms.Read<int>();
		}
		else
		{
			read_min_max_curve(ms, module.limit);
		}
		module.dampen = ms.Read<float>();
	}

	static void read_particle_inherit_velocity(MemoryStream& ms, ParticleSystem::InheritVelocityModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.mode = (ParticleSystemInheritVelocityMode) ms.Read<int>();
		read_min_max_curve(ms, module.curve);
	}

	static void read_particle_force_over_lifetime(MemoryStream& ms, ParticleSystem::ForceOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		read_min_max_curve(ms, module.x);
		read_min_max_curve(ms, module.y);
		read_min_max_curve(ms, module.z);
		module.space = (ParticleSystemSimulationSpace) ms.Read<int>();
		module.randomized = ms.Read<bool>();
	}

	static void read_particle_color_over_lifetime(MemoryStream& ms, ParticleSystem::ColorOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		read_min_max_gradient(ms, module.color);
	}

	static void read_particle_color_by_speed(MemoryStream& ms, ParticleSystem::ColorBySpeedModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		read_min_max_gradient(ms, module.color);
		module.range = ms.Read<Vector2>();
	}

	static void read_particle_size_over_lifetime(MemoryStream& ms, ParticleSystem::SizeOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.separate_axes = ms.Read<bool>();
		if (module.separate_axes)
		{
			read_min_max_curve(ms, module.x);
			read_min_max_curve(ms, module.y);
			read_min_max_curve(ms, module.z);
		}
		else
		{
			read_min_max_curve(ms, module.size);
		}
	}

	static void read_particle_size_by_speed(MemoryStream& ms, ParticleSystem::SizeBySpeedModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.separate_axes = ms.Read<bool>();
		if (module.separate_axes)
		{
			read_min_max_curve(ms, module.x);
			read_min_max_curve(ms, module.y);
			read_min_max_curve(ms, module.z);
		}
		else
		{
			read_min_max_curve(ms, module.size);
		}
		module.range = ms.Read<Vector2>();
	}

	static void read_particle_rotation_over_lifetime(MemoryStream& ms, ParticleSystem::RotationOverLifetimeModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.separate_axes = ms.Read<bool>();
		if (module.separate_axes)
		{
			read_min_max_curve(ms, module.x);
			read_min_max_curve(ms, module.y);
			read_min_max_curve(ms, module.z);
		}
		else
		{
			read_min_max_curve(ms, module.z);
		}
	}

	static void read_particle_rotation_by_speed(MemoryStream& ms, ParticleSystem::RotationBySpeedModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.separate_axes = ms.Read<bool>();
		if (module.separate_axes)
		{
			read_min_max_curve(ms, module.x);
			read_min_max_curve(ms, module.y);
			read_min_max_curve(ms, module.z);
		}
		else
		{
			read_min_max_curve(ms, module.z);
		}
		module.range = ms.Read<Vector2>();
	}

	static void read_particle_external_forces(MemoryStream& ms, ParticleSystem::ExternalForcesModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.multiplier = ms.Read<float>();
	}

	static void read_particle_texture_sheet_animation(MemoryStream& ms, ParticleSystem::TextureSheetAnimationModule& module)
	{
		module.enabled = ms.Read<bool>();
		if (!module.enabled)
		{
			return;
		}

		module.num_tiles_x = ms.Read<int>();
		module.num_tiles_y = ms.Read<int>();
		module.animation = (ParticleSystemAnimationType) ms.Read<int>();
		if (module.animation == ParticleSystemAnimationType::SingleRow)
		{
			module.use_random_row = ms.Read<bool>();
			module.row_index = ms.Read<int>();
		}

		read_min_max_curve(ms, module.frame_over_time);
		read_min_max_curve(ms, module.start_frame);
		module.cycle_count = ms.Read<int>();
		module.flip_u = ms.Read<float>();
		module.flip_v = ms.Read<float>();
		module.uv_channel_mask = ms.Read<int>();
	}

	static void read_particle_system(MemoryStream& ms, Ref<ParticleSystem>& com)
	{
		read_particle_main(ms, com->main);
		read_particle_emission(ms, com->emission);
		read_particle_shape(ms, com->shape);
		read_particle_velocity_over_lifetime(ms, com->velocity_over_lifetime);
		read_particle_limit_velocity_over_lifetime(ms, com->limit_velocity_over_lifetime);
		read_particle_inherit_velocity(ms, com->inherit_velocity);
		read_particle_force_over_lifetime(ms, com->force_over_lifetime);
		read_particle_color_over_lifetime(ms, com->color_over_lifetime);
		read_particle_color_by_speed(ms, com->color_by_speed);
		read_particle_size_over_lifetime(ms, com->size_over_lifetime);
		read_particle_size_by_speed(ms, com->size_by_speed);
		read_particle_rotation_over_lifetime(ms, com->rotation_over_lifetime);
		read_particle_rotation_by_speed(ms, com->rotation_by_speed);
		read_particle_external_forces(ms, com->external_forces);
		read_particle_texture_sheet_animation(ms, com->texture_sheet_animation);
	}

	static void read_particle_system_renderer(MemoryStream& ms, Ref<ParticleSystemRenderer>& com)
	{
		com->render_mode = (ParticleSystemRenderMode) ms.Read<int>();

		if (com->render_mode == ParticleSystemRenderMode::Stretch)
		{
			com->camera_velocity_scale = ms.Read<float>();
			com->velocity_scale = ms.Read<float>();
			com->length_scale = ms.Read<float>();
		}
		else if (com->render_mode == ParticleSystemRenderMode::Mesh)
		{
			auto mesh_path = read_string(ms);
			if (!mesh_path.Empty())
			{
				com->mesh = read_mesh(mesh_path);
			}
		}

		com->normal_direction = ms.Read<float>();
		com->sort_mode = (ParticleSystemSortMode) ms.Read<int>();
		com->sorting_fudge = ms.Read<float>();
		com->min_particle_size = ms.Read<float>();
		com->max_particle_size = ms.Read<float>();
		com->alignment = (ParticleSystemRenderSpace) ms.Read<int>();
		com->pivot = ms.Read<Vector3>();

		read_renderer_materials(ms, com.get());
	}

	static void read_terrain(MemoryStream& ms, Ref<Terrain>& com)
	{
		int lightmap_index = ms.Read<int>();
		Vector4 lightmap_scale_offset = ms.Read<Vector4>();
		Vector3 terrain_size = ms.Read<Vector3>();
		int heightmap_size = ms.Read<int>();

		Vector<float> heightmap_data(heightmap_size * heightmap_size);
		ms.Read(&heightmap_data[0], heightmap_data.SizeInBytes());

		int alphamap_size = ms.Read<int>();
		int alphamap_count = ms.Read<int>();

		Vector<Ref<Texture2D>> alphamaps(alphamap_count);
		for (int i = 0; i < alphamap_count; i++)
		{
			auto tex_path = read_string(ms);
			if (!tex_path.Empty())
			{
				alphamaps[i] = RefCast<Texture2D>(read_texture(tex_path));
			}
		}

		int splat_count = ms.Read<int>();
		Vector<TerrainSplatTexture> splat_textures(splat_count);
		for (int i = 0; i < splat_count; i++)
		{
			auto color_texture_path = read_string(ms);
			if (!color_texture_path.Empty())
			{
				splat_textures[i].texture = RefCast<Texture2D>(read_texture(color_texture_path));
			}
			auto normal_texture_path = read_string(ms);
			if (!normal_texture_path.Empty())
			{
				splat_textures[i].normal = RefCast<Texture2D>(read_texture(normal_texture_path));
			}
			splat_textures[i].tile_size = ms.Read<Vector2>();
			splat_textures[i].tile_offset = ms.Read<Vector2>();
		}

		com->SetLightmapIndex(lightmap_index);
		com->SetLightmapScaleOffset(lightmap_scale_offset);
		com->SetTerrainSize(terrain_size);
		com->SetHeightmapSize(heightmap_size);
		com->SetHeightmapData(heightmap_data);
		com->SetAlphamapSize(alphamap_size);
		com->SetAlphamaps(alphamaps);
		com->SetSplatTextures(splat_textures);
	}

	static Ref<Transform> read_transform(
		MemoryStream& ms,
		const Ref<Transform>& parent,
		FastList<Ref<GameObject>>& objs,
		Map<int, Ref<Transform>>& transform_instances)
	{
		auto name = read_string(ms);
		auto layer = ms.Read<int>();
		auto active = ms.Read<bool>();
		auto is_static = ms.Read<bool>();

		// 为了线程安全, 创建时不加入World
		auto obj = GameObject::Create(name, false);
		obj->SetLayer(layer);
		objs.AddLast(obj);
		obj->SetActive(active);
		obj->SetStatic(is_static);

		auto transform = obj->GetTransform();
		if (parent)
		{
			transform->SetParent(parent);
		}

		auto local_position = ms.Read<Vector3>();
		auto local_rotation = ms.Read<Quaternion>();
		auto local_scale = ms.Read<Vector3>();

		auto instance_id = ms.Read<int>();
		transform_instances.Add(instance_id, transform);

		transform->SetLocalPosition(local_position);
		transform->SetLocalRotation(local_rotation);
		transform->SetLocalScale(local_scale);

		int com_count = ms.Read<int>();

		for (int i = 0; i < com_count; i++)
		{
			auto component_name = read_string(ms);

			if (component_name == "MeshRenderer")
			{
				auto com = obj->AddComponent<MeshRenderer>();

				read_mesh_renderer(ms, com);
			}
			else if (component_name == "SkinnedMeshRenderer")
			{
				auto com = obj->AddComponent<SkinnedMeshRenderer>();

				read_skinned_mesh_renderer(ms, com, transform_instances);
			}
			else if (component_name == "Animation")
			{
				auto com = obj->AddComponent<Animation>();

				read_animation(ms, com);
			}
			else if (component_name == "ParticleSystemRenderer")
			{
				auto enabled = ms.Read<bool>();
				if (enabled)
				{
					auto com = obj->AddComponent<ParticleSystemRenderer>();

					read_particle_system_renderer(ms, com);
				}
			}
			else if (component_name == "ParticleSystem")
			{
				auto com = obj->AddComponent<ParticleSystem>();

				read_particle_system(ms, com);
			}
			else if (component_name == "Terrain")
			{
				auto com = obj->AddComponent<Terrain>();

				read_terrain(ms, com);
			}
			else if (component_name == "Canvas")
			{
				auto com = obj->AddComponent<UICanvasRenderer>();

				auto rect = RefCast<UIRect>(com);
				read_rect(ms, rect);
				read_canvas(ms, com);
			}
			else if (component_name == "Image")
			{
				auto com = obj->AddComponent<UISprite>();

				auto rect = RefCast<UIRect>(com);
				read_rect(ms, rect);
				read_image(ms, com);
			}
			else if (component_name == "Text")
			{
				auto com = obj->AddComponent<UILabel>();

				auto rect = RefCast<UIRect>(com);
				read_rect(ms, rect);
				read_label(ms, com);
			}
			else if (component_name == "RectTransform")
			{
				auto com = obj->AddComponent<UIView>();

				auto rect = RefCast<UIRect>(com);
				read_rect(ms, rect);
			}
		}

		auto child_count = ms.Read<int>();
		for (int i = 0; i < child_count; i++)
		{
			read_transform(ms, transform, objs, transform_instances);
		}

		auto anim = obj->GetComponent<Animation>();
		if (anim)
		{
			anim->FindBones();
		}

		return transform;
	}

	static Ref<GameObject> read_gameobject(const String& path, bool static_batch)
	{
		Ref<GameObject> obj;

		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return obj;
		}

		auto cache = Object::GetCache(path);
		if (cache)
		{
			obj = GameObject::Instantiate(RefCast<GameObject>(cache));
		}
		else
		{
			auto ms = MemoryStream(File::ReadAllBytes(full_path));

			FastList<Ref<GameObject>> objs;
			Map<int, Ref<Transform>> transform_instances;

			char flag[5] = { 0 };
			ms.Read(flag, 4);
			if (String(flag) == "VIRY")
			{
				unsigned int version = ms.Read<unsigned int>();
				if (version >= 0x00010000)
				{
					obj = read_transform(ms, Ref<Transform>(), objs, transform_instances)->GetGameObject();
				}
			}

#if VR_GLES
			if (obj && static_batch)
			{
				Renderer::BuildStaticBatch(obj);
			}
#endif

			if (obj)
			{
				Object::AddCache(path, obj);

				obj = GameObject::Instantiate(RefCast<GameObject>(obj));
			}

			ms.Close();
		}

		return obj;
	}

	void Resource::Init()
	{
		auto thread_init = []() {
			Graphics::GetDisplay()->CreateSharedContext();
		};
		auto thread_deinit = []() {
			Graphics::GetDisplay()->DestroySharedContext();
		};
		Vector<ThreadInfo> info;
		info.Add({ thread_init, thread_deinit });
		m_thread_res_load = RefMake<ThreadPool>(info);
	}

	void Resource::Deinit()
	{
		m_thread_res_load->Wait();
		m_thread_res_load.reset();
	}

	Ref<GameObject> Resource::LoadGameObject(const String& path, bool static_batch, LoadComplete callback)
	{
		auto obj = read_gameobject(path, static_batch);
		if (callback)
		{
			callback(obj);
		}
		return obj;
	}

	Ref<Texture> Resource::LoadTexture(const String& path)
	{
		return read_texture(path);
	}

	Ref<Font> Resource::LoadFont(const String& path)
	{
		return read_font(path);
	}

	Ref<Mesh> Resource::LoadMesh(const String& path)
	{
		return read_mesh(path);
	}

	void Resource::LoadLightmapSettings(const String& path)
	{
		String full_path = Application::DataPath() + path.Substring(String("Assets").Size());
		if (!File::Exist(full_path))
		{
			return;
		}

		auto ms = MemoryStream(File::ReadAllBytes(full_path));

		auto map_count = ms.Read<int>();

		Vector<Ref<Texture2D>> maps;
		for (int i = 0; i < map_count; i++)
		{
			auto tex_path = read_string(ms);
			if (!tex_path.Empty())
			{
				auto texture = read_texture(tex_path);
				if (texture)
				{
					maps.Add(RefCast<Texture2D>(texture));
				}
			}
		}
		LightmapSettings::SetLightmaps(maps);

		ms.Close();
	}

	void Resource::LoadGameObjectAsync(const String& path, bool static_batch, LoadComplete callback)
	{
		m_thread_res_load->AddTask(
		{
			[=]() {
			auto obj = Resource::LoadGameObject(path, static_batch, NULL);
			Graphics::GetDisplay()->FlushContext();
			return RefMake<Any>(obj);
		},
			[=](Ref<Any> any) {
			auto obj = any->Get<Ref<GameObject>>();
			if (callback)
			{
				callback(obj);
			}
		}
		}
		);
	}

	void Resource::LoadTextureAsync(const String& path, LoadComplete callback)
	{
		m_thread_res_load->AddTask(
		{
			[=]() {
			auto tex = Resource::LoadTexture(path);
			Graphics::GetDisplay()->FlushContext();
			return RefMake<Any>(tex);
		},
			[=](Ref<Any> any) {
			auto tex = any->Get<Ref<Texture>>();
			if (callback)
			{
				callback(tex);
			}
		}
		}
		);
	}

	void Resource::LoadFontAsync(const String& path, LoadComplete callback)
	{
		m_thread_res_load->AddTask(
		{
			[=]() {
			auto font = Resource::LoadFont(path);
			Graphics::GetDisplay()->FlushContext();
			return RefMake<Any>(font);
		},
			[=](Ref<Any> any) {
			auto font = any->Get<Ref<Font>>();
			if (callback)
			{
				callback(font);
			}
		}
		}
		);
	}

	void Resource::LoadMeshAsync(const String& path, LoadComplete callback)
	{
		m_thread_res_load->AddTask(
		{
			[=]() {
			auto mesh = Resource::LoadMesh(path);
			Graphics::GetDisplay()->FlushContext();
			return RefMake<Any>(mesh);
		},
			[=](Ref<Any> any) {
			auto mesh = any->Get<Ref<Mesh>>();
			if (callback)
			{
				callback(mesh);
			}
		}
		}
		);
	}
}
