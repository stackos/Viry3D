/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
            auto albedo = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet/Default_albedo.jpg",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto bump = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet/Default_normal.jpg",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto ao = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet/Default_AO.jpg",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto metal_roughness = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet/Default_metalRoughness.jpg",
                FilterMode::Linear,
                SamplerAddressMode::Repeat,
                false,
                false);
            auto emissive = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet/Default_emissive.jpg",
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
            auto brdf = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/babylon/environments/_environmentBRDFTexture.png",
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
            // Spherical Polynomial
            // Cubemap -> ConvertCubeMapTextureToSphericalPolynomial -> SphericalPolynomial
            {
                Vector3 x(0.057829572928459694f, 0.07247976674036509f, 0.11251884226746403f);
                Vector3 xx(0.5451950923281669f, 0.47561745973216946f, 0.5915887939770956f);
                Vector3 xy(0.0038382848502422824f, 0.010082420344840642f, 0.028773892052554595f);
                Vector3 y(0.20650985330812266f, 0.23137182050526978f, 0.4071420934841432f);
                Vector3 yy(0.5405465622452149f, 0.49248188682885485f, 0.6657606877012895f);
                Vector3 yz(0.011060956719164367f, 0.008238202668542628f, 0.010041904377146072f);
                Vector3 z(0.06524991987207855f, 0.03302735763549429f, 0.019678242745190016f);
                Vector3 zx(0.010204330390907297f, 0.01237347922146388f, 0.023365389794470153f);
                Vector3 zz(0.547528732463582f, 0.48271541737153467f, 0.6211293957837286f);

                material->SetVector("vSphericalX", x);
                material->SetVector("vSphericalY", y);
                material->SetVector("vSphericalZ", z);
                material->SetVector("vSphericalXX_ZZ", xx - zz);
                material->SetVector("vSphericalYY_ZZ", yy - zz);
                material->SetVector("vSphericalZZ", zz);
                material->SetVector("vSphericalXY", xy);
                material->SetVector("vSphericalYZ", yz);
                material->SetVector("vSphericalZX", zx);
            }
            // light0
            {
                material->SetVector("vLightData", Vector4(-1, -1, 1, 1));
                material->SetVector("vLightDiffuse", Vector4(1, 1, 1, 0.00001f));
                material->SetVector("vLightSpecular", Vector4(1, 1, 1, 1));
            }

            auto mesh = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/babylon/glTF/DamagedHelmet.mesh");

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
