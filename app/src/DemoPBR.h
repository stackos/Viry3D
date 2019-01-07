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

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Light.h"
#include "graphics/CubeMapToSphericalPolynomialTools.h"
#include "io/File.h"
#include "json/json.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"
#include "Resources.h"

namespace Viry3D
{
    class DemoPBR : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Vector3 rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 0, -3.0f),
            Vector3(0, 0, 0),
            45,
            0.3f,
            1000
        };
        struct LightParam
        {
            Color ambient_color;
            Color light_color;
            float light_intensity;
        };
        LightParam m_light_param = {
            Color(0.2f, 0.2f, 0.2f, 1),
            Color(1, 1, 1, 1),
            0.8f
        };

        Camera* m_camera = nullptr;
        Camera* m_ui_camera = nullptr;
        Label* m_label = nullptr;
        Ref<Light> m_light;
        float m_rot_y = 0;
        Ref<MeshRenderer> m_renderer;

        void InitCamera()
        {
            m_camera = Display::Instance()->CreateCamera();
            m_camera->SetDepth(0);
            m_camera->SetLocalPosition(m_camera_param.pos);
            m_camera->SetLocalRotation(Quaternion::Euler(m_camera_param.rot));

            m_camera->SetFieldOfView(m_camera_param.fov);
            m_camera->SetNearClip(m_camera_param.near_clip);
            m_camera->SetFarClip(m_camera_param.far_clip);
        }

        void InitLight()
        {
            m_light = RefMake<Light>(LightType::Directional);
            m_light->SetLocalRotation(Quaternion::Euler(45, 60, 0));
            m_light->SetAmbientColor(m_light_param.ambient_color);
            m_light->SetColor(m_light_param.light_color);
            m_light->SetIntensity(m_light_param.light_intensity);
        }

        void InitMesh()
        {
#if VR_WINDOWS
            auto albedo = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/albedo_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
            auto bump = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/normal_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
            auto ao = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/ao_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
            auto metal_roughness = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/metal_roughness_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
            auto emissive = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/emissive_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
            auto cubemap = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/texture/env/prefilter/prefilter_bc1_rgba.ktx",
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge,
                false);
#else
#if VR_ANDROID
            auto albedo = Texture::LoadFromKTXFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/albedo_etc2_rgb.ktx",
                //Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/albedo_astc_4x4.ktx",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false);
#else
            auto albedo = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/albedo.png",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
#endif
            auto bump = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/normal.png",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto ao = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/ao.png",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto metal_roughness = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/metal_roughness.png",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto emissive = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/emissive.png",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto cubemap = Texture::CreateCubemap(1024, TextureFormat::R8G8B8A8, FilterMode::Linear, SamplerAddressMode::ClampToEdge, true);
            for (int i = 0; i < 11; ++i)
            {
                for (int j = 0; j < 6; ++j)
                {
                    int width;
                    int height;
                    int bpp;
                    ByteBuffer pixels = Texture::LoadImageFromFile(String::Format((Application::Instance()->GetDataPath() + "/texture/env/prefilter/%d_%d.png").CString(), i, j), width, height, bpp);
                    cubemap->UpdateCubemap(pixels, (CubemapFace) j, i);
                }
            }
#endif
            auto brdf = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/texture/env/brdf.png",
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge,
                false,
                false);

            RenderState render_state;
            auto shader = RefMake<Shader>(
                "",
                Vector<String>({ "../pbr.vs" }),
                "",
                "",
                Vector<String>({ "../pbr.fs" }),
                "",
                render_state);
            auto material = RefMake<Material>(shader);
            material->SetColor("vAlbedoColor", Color(1, 1, 1, 1));
            material->SetColor("vReflectivityColor", Color(1, 1, 0, 0));
            material->SetColor("vReflectionColor", Color(1, 1, 1, 1));
            material->SetColor("vAmbientColor", Color(0, 0, 0, 0));
            material->SetColor("vEmissiveColor", Color(1, 1, 1, 1));
            material->SetVector("vAlbedoInfos", Vector4(0, 1, 0, 0));
            material->SetVector("vBumpInfos", Vector4(0, 1, 0.05f, 0));
            material->SetVector("vAmbientInfos", Vector4(0, 1, 1, 1));
            material->SetVector("vTangentSpaceParams", Vector4(-1, 1, 0, 0));
            material->SetVector("vReflectionMicrosurfaceInfos", Vector4(128, 1.5f, 0, 0));
            material->SetVector("vReflectionInfos", Vector4(1, 0, 0, 0));
            material->SetVector("vLightingIntensity", Vector4(1, 1, 1, 1));
            material->SetVector("vEmissiveInfos", Vector4(0, 1, 0, 0));
            material->SetTexture("albedoSampler", albedo);
            material->SetTexture("bumpSampler", bump);
            material->SetTexture("ambientSampler", ao);
            material->SetTexture("reflectivitySampler", metal_roughness);
            material->SetTexture("reflectionSampler", cubemap);
            material->SetTexture("environmentBrdfSampler", brdf);
            material->SetTexture("emissiveSampler", emissive);
            material->SetMatrix("reflectionMatrixVS", Matrix4x4::Identity());
            material->SetMatrix("reflectionMatrixFS", Matrix4x4::Identity());
            material->SetFloat("exposureLinear", 0.8f);
            material->SetFloat("contrast", 1.2f);
            // spherical polynomial
            {
                SphericalPolynomial sp;

                String sp_json = File::ReadAllText(Application::Instance()->GetDataPath() + "/texture/env/prefilter/spherical_polynomial.json");
                auto reader = Ref<Json::CharReader>(Json::CharReaderBuilder().newCharReader());
                Json::Value root;
                const char* begin = sp_json.CString();
                const char* end = begin + sp_json.Size();
                if (reader->parse(begin, end, &root, nullptr))
                {
                    sp.x = Vector3(root["x"][0].asFloat(), root["x"][1].asFloat(), root["x"][2].asFloat());
                    sp.y = Vector3(root["y"][0].asFloat(), root["y"][1].asFloat(), root["y"][2].asFloat());
                    sp.z = Vector3(root["z"][0].asFloat(), root["z"][1].asFloat(), root["z"][2].asFloat());
                    sp.xx = Vector3(root["xx"][0].asFloat(), root["xx"][1].asFloat(), root["xx"][2].asFloat());
                    sp.yy = Vector3(root["yy"][0].asFloat(), root["yy"][1].asFloat(), root["yy"][2].asFloat());
                    sp.zz = Vector3(root["zz"][0].asFloat(), root["zz"][1].asFloat(), root["zz"][2].asFloat());
                    sp.xy = Vector3(root["xy"][0].asFloat(), root["xy"][1].asFloat(), root["xy"][2].asFloat());
                    sp.yz = Vector3(root["yz"][0].asFloat(), root["yz"][1].asFloat(), root["yz"][2].asFloat());
                    sp.zx = Vector3(root["zx"][0].asFloat(), root["zx"][1].asFloat(), root["zx"][2].asFloat());
                }
                
                material->SetVector("vSphericalX", sp.x);
                material->SetVector("vSphericalY", sp.y);
                material->SetVector("vSphericalZ", sp.z);
                material->SetVector("vSphericalXX_ZZ", sp.xx - sp.zz);
                material->SetVector("vSphericalYY_ZZ", sp.yy - sp.zz);
                material->SetVector("vSphericalZZ", sp.zz);
                material->SetVector("vSphericalXY", sp.xy);
                material->SetVector("vSphericalYZ", sp.yz);
                material->SetVector("vSphericalZX", sp.zx);
            }
            // light0
            {
                material->SetVector("vLightData", Vector4(-1, -1, 1, 1));
                material->SetVector("vLightDiffuse", Vector4(1, 1, 1, 0.00001f));
                material->SetVector("vLightSpecular", Vector4(1, 1, 1, 1));
            }

            auto mesh = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/res/model/DamagedHelmet/DamagedHelmet.mesh");

            auto renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(mesh);
            renderer->SetLocalRotation(Quaternion::Euler(90, 0, 0));
            m_camera->AddRenderer(renderer);

            m_renderer = renderer;
        }

        void InitUI()
        {
            m_ui_camera = Display::Instance()->CreateCamera();
            m_ui_camera->SetDepth(1);
            m_ui_camera->SetClearFlags(CameraClearFlags::Nothing);

            auto canvas = RefMake<CanvasRenderer>();
            m_ui_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::Consola));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitLight();
            this->InitMesh();
            this->InitUI();
        }

        virtual void Done()
        {
            m_renderer.reset();
            if (m_ui_camera)
            {
                Display::Instance()->DestroyCamera(m_ui_camera);
                m_ui_camera = nullptr;
            }
            if (m_camera)
            {
                Display::Instance()->DestroyCamera(m_camera);
                m_camera = nullptr;
            }
        }

        virtual void Update()
        {
            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }

            if (m_renderer)
            {
                m_rot_y += 0.5f;
                m_renderer->SetLocalRotation(Quaternion::Euler(90, m_rot_y, 0));
            }
        }
    };
}
