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

#include "Resources.h"
#include "Engine.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "graphics/MeshRenderer.h"
#include "graphics/SkinnedMeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Image.h"
#include "graphics/Texture.h"
#include "animation/Animation.h"
#include "json/json.h"
#include "physics/SpringBone.h"
#include "physics/SpringCollider.h"
#include "physics/SpringManager.h"

namespace Viry3D
{
	static Map<String, Ref<Object>> g_cache;

	void Resources::Init()
	{
	
	}

	void Resources::Done()
	{
		g_cache.Clear();
	}

    static String ReadString(MemoryStream& ms)
    {
        int size = ms.Read<int>();
        return ms.ReadString(size);
    }

    static Ref<Texture> ReadTexture(const String& path)
    {
        if (g_cache.Contains(path))
        {
            return RefCast<Texture>(g_cache[path]);
        }

        Ref<Texture> texture;

        String full_path = Engine::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            String json = File::ReadAllText(full_path);

            auto reader = Ref<Json::CharReader>(Json::CharReaderBuilder().newCharReader());
            Json::Value root;
            const char* begin = json.CString();
            const char* end = begin + json.Size();
            if (reader->parse(begin, end, &root, nullptr))
            {
                String texture_name = root["name"].asCString();
                int width = root["width"].asInt();
                int height = root["height"].asInt();
                SamplerAddressMode wrap_mode = (SamplerAddressMode) root["wrap_mode"].asInt();
                FilterMode filter_mode = (FilterMode) root["filter_mode"].asInt();
                String texture_type = root["type"].asCString();

                if (texture_type == "Texture2D")
                {
                    int mipmap_count = root["mipmap"].asInt();
                    String png_path = root["path"].asCString();

                    texture = Texture::LoadTexture2DFromFile(Engine::Instance()->GetDataPath() + "/" + png_path, filter_mode, wrap_mode, mipmap_count > 1);
                    texture->SetName(texture_name);
                }
                else if (texture_type == "Cubemap")
                {
                    int mipmap_count = root["mipmap"].asInt();
                    Json::Value levels = root["levels"];

                    assert(width == height);
                    
                    texture = Texture::CreateCubemap(width, TextureFormat::R8G8B8A8, filter_mode, wrap_mode, mipmap_count > 1);

                    for (int i = 0; i < mipmap_count; ++i)
                    {
                        Json::Value faces = levels[i];
						ByteBuffer buffer;
						Vector<int> offsets(6);

                        for (int j = 0; j < 6; ++j)
                        {
                            String face_path = faces[j].asCString();

                            Ref<Image> image = Image::LoadFromFile(Engine::Instance()->GetDataPath() + "/" + face_path);
							if (image)
							{
								if (buffer.Size() == 0)
								{
									buffer = ByteBuffer(image->data.Size() * 6);
								}
								Memory::Copy(&buffer[j * image->data.Size()], image->data.Bytes(), image->data.Size());
								offsets[j] = j * image->data.Size();
							}
                        }

						if (buffer.Size() > 0)
						{
							texture->UpdateCubemap(buffer, i, offsets);
						}
                    }
                }
            }
        }

		g_cache.Add(path, texture);

        return texture;
    }

    static Ref<Material> ReadMaterial(const String& path)
    {
        if (g_cache.Contains(path))
        {
            return RefCast<Material>(g_cache[path]);
        }

        Ref<Material> material;

        String full_path = Engine::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            MemoryStream ms(File::ReadAllBytes(full_path));

            String material_name = ReadString(ms);
            String shader_name = ReadString(ms);
            
            int keyword_count = ms.Read<int>();
            Vector<String> keywords;
            for (int i = 0; i < keyword_count; ++i)
            {
                String keyword = ReadString(ms);
                keywords.Add(keyword);
            }
            
            Ref<Shader> shader = Shader::Find(shader_name, keywords);
            if (shader)
            {
                material = RefMake<Material>(shader);
                material->SetName(material_name);
            }
            
            int property_count = ms.Read<int>();
            for (int i = 0; i < property_count; ++i)
            {
                String property_name = ReadString(ms);
                MaterialProperty::Type property_type = (MaterialProperty::Type) ms.Read<int>();

                switch (property_type)
                {
                    case MaterialProperty::Type::Color:
                    {
						byte c[4];
                        ms.Read(c, sizeof(c));
						Color value(c[0] / 255.0f, c[1] / 255.0f, c[2] / 255.0f, c[3] / 255.0f);
                        if (material)
                        {
                            material->SetColor(property_name, value);
                        }
                        break;
                    }
                    case MaterialProperty::Type::Vector:
                    {
                        Vector4 value = ms.Read<Vector4>();
                        if (material)
                        {
                            material->SetVector(property_name, value);
                        }
                        break;
                    }
                    case MaterialProperty::Type::Float:
                    case MaterialProperty::Type::Range:
                    {
                        float value = ms.Read<float>();
                        if (material)
                        {
                            material->SetFloat(property_name, value);
                        }
                        break;
                    }
                    case MaterialProperty::Type::Texture:
                    {
                        Vector4 uv_scale_offset = ms.Read<Vector4>();
                        (void) uv_scale_offset;
                        
                        String texture_path = ReadString(ms);
                        if (texture_path.Size() > 0)
                        {
                            Ref<Texture> texture = ReadTexture(texture_path);
                            if (material && texture)
                            {
                                material->SetTexture(property_name, texture);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

		g_cache.Add(path, material);

        return material;
    }

    static void ReadRenderer(MemoryStream& ms, const Ref<Renderer>& renderer)
    {
        int lightmap_index = ms.Read<int>();
        Vector4 lightmap_scale_offset = ms.Read<Vector4>();
        bool cast_shadow = ms.Read<byte>() == 1;
        bool receive_shadow = ms.Read<byte>() == 1;

        (void) cast_shadow;
        (void) receive_shadow;
        
        int material_count = ms.Read<int>();
		Vector<Ref<Material>> materials(material_count);
        for (int i = 0; i < material_count; ++i)
        {
            String material_path = ReadString(ms);
            if (material_path.Size() > 0)
            {
				materials[i] = ReadMaterial(material_path);
            }
        }
		renderer->SetMaterials(materials);

        if (lightmap_index >= 0)
        {
            renderer->SetLightmapIndex(lightmap_index);
            renderer->SetLightmapScaleOffset(lightmap_scale_offset);
        }
    }

	static Ref<Mesh> ReadMesh(const String& path)
	{
		if (g_cache.Contains(path))
		{
			return RefCast<Mesh>(g_cache[path]);
		}

		Ref<Mesh> mesh = Mesh::LoadFromFile(Engine::Instance()->GetDataPath() + "/" + path);

		g_cache.Add(path, mesh);

		return mesh;
	}

    static void ReadMeshRenderer(MemoryStream& ms, const Ref<MeshRenderer>& renderer)
    {
        ReadRenderer(ms, renderer);

        String mesh_path = ReadString(ms);
		if (mesh_path.Size() > 0)
		{
			auto mesh = ReadMesh(mesh_path);
			renderer->SetMesh(mesh);
		}
    }

    static void ReadSkinnedMeshRenderer(MemoryStream& ms, const Ref<SkinnedMeshRenderer>& renderer)
    {
        ReadMeshRenderer(ms, renderer);

        int bone_count = ms.Read<int>();

        Vector<String> bones(bone_count);
        for (int i = 0; i < bone_count; ++i)
        {
            bones[i] = ReadString(ms);
        }
        renderer->SetBonePaths(bones);
    }

    static void ReadAnimationCurve(MemoryStream& ms, AnimationCurve* curve)
    {
        int key_count = ms.Read<int>();
        for (int k = 0; k < key_count; ++k)
        {
            float time = ms.Read<float>();
            float value = ms.Read<float>();
            float in_tangent = ms.Read<float>();
            float out_tangent = ms.Read<float>();
            
            curve->AddKey(time, value, in_tangent, out_tangent);
        }
    }
    
	static Ref<AnimationClip> ReadAnimationClip(const String& path)
	{
		if (g_cache.Contains(path))
		{
			return RefCast<AnimationClip>(g_cache[path]);
		}

		Ref<AnimationClip> clip;

		String full_path = Engine::Instance()->GetDataPath() + "/" + path;
		if (File::Exist(full_path))
		{
			MemoryStream ms(File::ReadAllBytes(full_path));

			clip = RefMake<AnimationClip>();

			String clip_name = ReadString(ms);
			float clip_length = ms.Read<float>();
			float clip_fps = ms.Read<float>();
			int clip_wrap_mode = ms.Read<int>();
			int curve_count = ms.Read<int>();

			clip->name = clip_name;
			clip->length = clip_length;
			clip->fps = clip_fps;
			clip->wrap_mode = (AnimationWrapMode) clip_wrap_mode;

			for (int j = 0; j < curve_count; ++j)
			{
				String curve_path = ReadString(ms);
				AnimationCurvePropertyType property_type = (AnimationCurvePropertyType) ms.Read<int>();
				String property_name = ReadString(ms);

				AnimationCurveWrapper* curve = nullptr;
				for (int k = 0; k < clip->curves.Size(); ++k)
				{
					if (clip->curves[k].path == curve_path)
					{
						curve = &clip->curves[k];
						break;
					}
				}
				if (curve == nullptr)
				{
					AnimationCurveWrapper new_path_curve;
					new_path_curve.path = curve_path;
					clip->curves.Add(new_path_curve);
					curve = &clip->curves[clip->curves.Size() - 1];
				}

				AnimationCurveProperty property;
				property.type = property_type;
				property.name = property_name;

				curve->properties.Add(property);

				AnimationCurve* anim_curve = &curve->properties[curve->properties.Size() - 1].curve;
                ReadAnimationCurve(ms, anim_curve);
			}
		}

		g_cache.Add(path, clip);

		return clip;
	}

    static void ReadAnimation(MemoryStream& ms, const Ref<Animation>& animation)
    {
        int clip_count = ms.Read<int>();

        Vector<Ref<AnimationClip>> clips(clip_count);

        for (int i = 0; i < clip_count; ++i)
        {
			String clip_path = ReadString(ms);
			if (clip_path.Size() > 0)
			{
				clips[i] = ReadAnimationClip(clip_path);
			}
        }

        animation->SetClips(clips);
    }
    
    static void ReadSpringBone(MemoryStream& ms, const Ref<SpringBone>& bone)
    {
        bone->child_name = ReadString(ms);
        bone->radius = ms.Read<float>();
        bone->stiffness_force = ms.Read<float>();
        bone->drag_force = ms.Read<float>();
        bone->threshold = ms.Read<float>();
        bone->bone_axis = ms.Read<Vector3>();
        bone->spring_force = ms.Read<Vector3>();
        int collider_count = ms.Read<int>();
        bone->collider_paths.Resize(collider_count);
        for (int i = 0; i < collider_count; ++i)
        {
            bone->collider_paths[i] = ReadString(ms);
        }
    }
    
    static void ReadSpringManager(MemoryStream& ms, const Ref<SpringManager>& manager)
    {
        manager->dynamic_ratio = ms.Read<float>();
        manager->stiffness_force = ms.Read<float>();
        ReadAnimationCurve(ms, &manager->stiffness_curve);
        manager->drag_force = ms.Read<float>();
        ReadAnimationCurve(ms, &manager->drag_curve);
        int bone_count = ms.Read<int>();
        manager->bone_paths.Resize(bone_count);
        for (int i = 0; i < bone_count; ++i)
        {
            manager->bone_paths[i] = ReadString(ms);
        }
    }

    static Ref<GameObject> ReadGameObject(MemoryStream& ms, const Ref<GameObject>& parent)
    {
        String name = ReadString(ms);
        int layer = ms.Read<int>();
        bool active = ms.Read<byte>() == 1;
		Vector3 local_pos = ms.Read<Vector3>();
		Quaternion local_rot = ms.Read<Quaternion>();
		Vector3 local_scale = ms.Read<Vector3>();

		Ref<GameObject> obj = GameObject::Create(name);
		obj->SetLayer(layer);
		obj->SetActive(active);

		if (parent)
		{
			obj->GetTransform()->SetParent(parent->GetTransform());
		}
		obj->GetTransform()->SetLocalPosition(local_pos);
		obj->GetTransform()->SetLocalRotation(local_rot);
		obj->GetTransform()->SetLocalScale(local_scale);

        int com_count = ms.Read<int>();
        for (int i = 0; i < com_count; ++i)
        {
            String com_name = ReadString(ms);

            if (com_name == "MeshRenderer")
            {
				auto com = obj->AddComponent<MeshRenderer>();
                ReadMeshRenderer(ms, com);
            }
            else if (com_name == "SkinnedMeshRenderer")
            {
				auto com = obj->AddComponent<SkinnedMeshRenderer>();
                ReadSkinnedMeshRenderer(ms, com);

                if (parent)
                {
                    com->SetBonesRoot(parent->GetTransform()->GetRoot());
                }
                else
                {
                    com->SetBonesRoot(obj->GetTransform());
                }
            }
            else if (com_name == "Animation")
            {
				auto com = obj->AddComponent<Animation>();
                ReadAnimation(ms, com);
            }
            else if (com_name == "SpringBone")
            {
                auto com = obj->AddComponent<SpringBone>();
                ReadSpringBone(ms, com);
            }
            else if (com_name == "SpringCollider")
            {
                auto com = obj->AddComponent<SpringCollider>();
                com->radius = ms.Read<float>();
            }
            else if (com_name == "SpringManager")
            {
                auto com = obj->AddComponent<SpringManager>();
                ReadSpringManager(ms, com);
            }
        }

		int child_count = ms.Read<int>();
		for (int i = 0; i < child_count; ++i)
		{
			ReadGameObject(ms, obj);
		}

        return obj;
    }

    Ref<GameObject> Resources::LoadGameObject(const String& path)
    {
		Ref<GameObject> obj;

        String full_path = Engine::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            MemoryStream ms(File::ReadAllBytes(full_path));

			obj = ReadGameObject(ms, Ref<GameObject>());
        }

        return obj;
    }

	Ref<Mesh> Resources::LoadMesh(const String& path)
	{
		Ref<Mesh> mesh;

		mesh = ReadMesh(path);

		return mesh;
	}

    Ref<Texture> Resources::LoadTexture(const String& path)
    {
        Ref<Texture> texture;

        texture = ReadTexture(path);

        return texture;
    }

    Ref<Texture> Resources::LoadLightmap(const String& path)
    {
		if (g_cache.Contains(path))
		{
			return RefCast<Texture>(g_cache[path]);
		}

        Ref<Texture> lightmap;
        
        String full_path = Engine::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            MemoryStream ms(File::ReadAllBytes(full_path));

            Vector<Ref<Texture>> textures;
            int texture_size = 0;

            int lightmap_count = ms.Read<int>();
            for (int i = 0; i < lightmap_count; ++i)
            {
                String texture_path = ReadString(ms);
                assert(texture_path.Size() > 0);

                auto texture = ReadTexture(texture_path);
                assert(texture);

                if (texture->GetWidth() > texture_size)
                {
                    texture_size = texture->GetWidth();
                }

                textures.Add(texture);
            }

            if (lightmap_count > 0)
            {
				/*
                Vector<ByteBuffer> pixels(lightmap_count);
                for (int i = 0; i < lightmap_count; ++i)
                {
                    pixels[i] = ByteBuffer(texture_size * texture_size * 4);
                    
                    if (textures[i]->GetWidth() < texture_size)
                    {
                        // resize texture same to the max one
                        auto temp = Texture::CreateTexture2DFromMemory(
                            pixels[i],
                            texture_size, texture_size,
                            TextureFormat::R8G8B8A8,
                            FilterMode::Linear,
                            SamplerAddressMode::ClampToEdge,
                            false);
                        temp->CopyTexture(
                            textures[i],
                            0, 0,
                            0, 0,
                            textures[i]->GetWidth(), textures[i]->GetHeight(),
                            0, 0,
                            0, 0,
                            texture_size, texture_size);
                        temp->CopyToMemory(pixels[i], 0, 0);
                    }
                    else
                    {
                        textures[i]->CopyToMemory(pixels[i], 0, 0);
                    }
                }

                lightmap = Texture::CreateTexture2DArrayFromMemory(
                    pixels,
                    texture_size, texture_size,
                    lightmap_count,
                    TextureFormat::R8G8B8A8,
                    FilterMode::Linear,
                    SamplerAddressMode::ClampToEdge,
                    false,
                    false);
				*/
            }
        }

		g_cache.Add(path, lightmap);

        return lightmap;
    }
}
