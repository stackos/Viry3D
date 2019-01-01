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
#include "Node.h"
#include "Application.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "graphics/MeshRenderer.h"
#include "graphics/SkinnedMeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "animation/Animation.h"
#include "json/json.h"

namespace Viry3D
{
    static Map<String, Ref<Object>> g_loading_cache;

    static String ReadString(MemoryStream& ms)
    {
        int size = ms.Read<int>();
        return ms.ReadString(size);
    }

    static Ref<Texture> ReadTexture(const String& path)
    {
        if (g_loading_cache.Contains(path))
        {
            return RefCast<Texture>(g_loading_cache[path]);
        }

        Ref<Texture> texture;

        String full_path = Application::Instance()->GetDataPath() + "/" + path;
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

                    texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/" + png_path, filter_mode, wrap_mode, mipmap_count > 1, false);
                    texture->SetName(texture_name);
                }
                else if (texture_type == "Cubemap")
                {
                    int mipmap_count = root["mipmap"].asInt();
                    Json::Value levels = root["levels"];

                    texture = Texture::CreateCubemap(width, TextureFormat::R8G8B8A8, filter_mode, wrap_mode, mipmap_count > 1);

                    for (int i = 0; i < mipmap_count; ++i)
                    {
                        Json::Value faces = levels[i];

                        for (int j = 0; j < 6; ++j)
                        {
                            String face_path = faces[j].asCString();

                            int w;
                            int h;
                            int bpp;
                            ByteBuffer pixels = Texture::LoadImageFromFile(Application::Instance()->GetDataPath() + "/" + face_path, w, h, bpp);
                            texture->UpdateCubemap(pixels, (CubemapFace) j, i);
                        }
                    }
                }
            }
        }

        g_loading_cache.Add(path, texture);

        return texture;
    }

    static Ref<Material> ReadMaterial(const String& path)
    {
        if (g_loading_cache.Contains(path))
        {
            return RefCast<Material>(g_loading_cache[path]);
        }

        Ref<Material> material;

        String full_path = Application::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            MemoryStream ms(File::ReadAllBytes(full_path));

            String material_name = ReadString(ms);
            String shader_name = ReadString(ms);
            int property_count = ms.Read<int>();

            Ref<Shader> shader = Shader::Find(shader_name);
            if (shader)
            {
                material = RefMake<Material>(shader);
                material->SetName(material_name);
            }

            for (int i = 0; i < property_count; ++i)
            {
                String property_name = ReadString(ms);
                MaterialProperty::Type property_type = (MaterialProperty::Type) ms.Read<int>();

                switch (property_type)
                {
                    case MaterialProperty::Type::Color:
                    {
                        Color value = ms.Read<Color>();
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
                }
            }
        }

        g_loading_cache.Add(path, material);

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
        for (int i = 0; i < material_count; ++i)
        {
            String material_path = ReadString(ms);
            if (material_path.Size() > 0)
            {
                Ref<Material> material = ReadMaterial(material_path);
                if (material)
                {
                    renderer->SetMaterial(material);
                }
            }
        }

        if (lightmap_index >= 0)
        {
            renderer->SetLightmapIndex(lightmap_index);
            renderer->SetLightmapScaleOffset(lightmap_scale_offset);
        }
    }

    static void ReadMeshRenderer(MemoryStream& ms, const Ref<MeshRenderer>& renderer)
    {
        ReadRenderer(ms, renderer);

        String mesh_path = ReadString(ms);
        auto mesh = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/" + mesh_path);
        renderer->SetMesh(mesh);
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

    static void ReadAnimation(MemoryStream& ms, const Ref<Animation>& animation)
    {
        int clip_count = ms.Read<int>();

        Vector<AnimationClip> clips(clip_count);

        for (int i = 0; i < clip_count; ++i)
        {
            String clip_name = ReadString(ms);
            float clip_length = ms.Read<float>();
            float clip_fps = ms.Read<float>();
            int clip_wrap_mode = ms.Read<int>();
            int curve_count = ms.Read<int>();

            AnimationClip& clip = clips[i];
            clip.name = clip_name;
            clip.length = clip_length;
            clip.fps = clip_fps;
            clip.wrap_mode = (AnimationWrapMode) clip_wrap_mode;

            for (int j = 0; j < curve_count; ++j)
            {
                String curve_path = ReadString(ms);
                int property_type = ms.Read<int>();
                int key_count = ms.Read<int>();

                AnimationCurveWrapper* curve = nullptr;
                for (int k = 0; k < clip.curves.Size(); ++k)
                {
                    if (clip.curves[k].path == curve_path)
                    {
                        curve = &clip.curves[k];
                        break;
                    }
                }
                if (curve == nullptr)
                {
                    AnimationCurveWrapper new_path_curve;
                    new_path_curve.path = curve_path;
                    clip.curves.Add(new_path_curve);
                    curve = &clip.curves[clip.curves.Size() - 1];
                }
                
                curve->property_types.Add((CurvePropertyType) property_type);
                curve->curves.Add(AnimationCurve());

                AnimationCurve* anim_curve = &curve->curves[curve->curves.Size() - 1];

                for (int k = 0; k < key_count; ++k)
                {
                    float time = ms.Read<float>();
                    float value = ms.Read<float>();
                    float in_tangent = ms.Read<float>();
                    float out_tangent = ms.Read<float>();

                    anim_curve->AddKey(time, value, in_tangent, out_tangent);
                }
            }
        }

        animation->SetClips(std::move(clips));
    }

    static Ref<Node> ReadNode(MemoryStream& ms, const Ref<Node>& parent)
    {
        Ref<Node> node;

        String name = ReadString(ms);
        int layer = ms.Read<int>();
        bool active = ms.Read<byte>() == 1;

        (void) layer;
        (void) active;

        Vector3 local_pos = ms.Read<Vector3>();
        Quaternion local_rot = ms.Read<Quaternion>();
        Vector3 local_scale = ms.Read<Vector3>();

        int com_count = ms.Read<int>();
        for (int i = 0; i < com_count; ++i)
        {
            String com_name = ReadString(ms);

            if (com_name == "MeshRenderer")
            {
                assert(!node);

                auto com = RefMake<MeshRenderer>();
                ReadMeshRenderer(ms, com);
                node = com;
            }
            else if (com_name == "SkinnedMeshRenderer")
            {
                assert(!node);

                auto com = RefMake<SkinnedMeshRenderer>();
                ReadSkinnedMeshRenderer(ms, com);
                node = com;

                if (parent)
                {
                    com->SetBonesRoot(Node::GetRoot(parent));
                }
                else
                {
                    com->SetBonesRoot(com);
                }
            }
            else if (com_name == "Animation")
            {
                assert(!node);

                auto com = RefMake<Animation>();
                ReadAnimation(ms, com);
                node = com;
            }
        }

        if (!node)
        {
            node = RefMake<Node>();
        }

        if (parent)
        {
            Node::SetParent(node, parent);
        }

        node->SetName(name);
        node->SetLocalPosition(local_pos);
        node->SetLocalRotation(local_rot);
        node->SetLocalScale(local_scale);

        int child_count = ms.Read<int>();
        for (int i = 0; i < child_count; ++i)
        {
            ReadNode(ms, node);
        }

        return node;
    }

    Ref<Node> Resources::LoadNode(const String& path)
    {
        Ref<Node> node;

        String full_path = Application::Instance()->GetDataPath() + "/" + path;
        if (File::Exist(full_path))
        {
            MemoryStream ms(File::ReadAllBytes(full_path));

            node = ReadNode(ms, Ref<Node>());

            g_loading_cache.Clear();
        }

        return node;
    }

    Ref<Texture> Resources::LoadTexture(const String& path)
    {
        Ref<Texture> texture;

        texture = ReadTexture(path);

        g_loading_cache.Clear();

        return texture;
    }

    Ref<Texture> Resources::LoadLightmap(const String& path)
    {
        Ref<Texture> lightmap;
        
        String full_path = Application::Instance()->GetDataPath() + "/" + path;
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
                            false,
                            false,
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
            }

            g_loading_cache.Clear();
        }

        return lightmap;
    }
}
