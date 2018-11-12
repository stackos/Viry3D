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
    class DemoSSAO : public Demo
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
            Vector3(0, 5, -10),
            Vector3(30, 0, 0),
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
        Camera* m_blit_color_camera = nullptr;

        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                1,
                true,
                FilterMode::Nearest,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            /*
            auto color_texture_2 = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                1,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetExtraRenderTargets({ color_texture_2 });
            */

            // color -> window
            m_blit_color_camera = Display::Instance()->CreateBlitCamera(1, color_texture);

            m_ui_camera->SetDepth(2);
        }

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

        void InitScene()
        {
            RenderState render_state;
            auto shader = RefMake<Shader>(
                "",
                Vector<String>({ "Diffuse.vs.in" }),
                "",
                "",
                Vector<String>({ "Diffuse.fs.in" }),
                "",
                render_state);
            Shader::AddCache("Diffuse", shader);

            auto node = Resources::LoadNode("res/scene/lightmap/scene.go");
            for (int i = 0; i < node->GetChildCount(); ++i)
            {
                Ref<MeshRenderer> mesh_renderer = RefCast<MeshRenderer>(node->GetChild(i));
                if (mesh_renderer)
                {
                    m_camera->AddRenderer(mesh_renderer);

                    auto material = mesh_renderer->GetMaterial();
                    if (material)
                    {
                        if (!material->GetTexture("u_texture"))
                        {
                            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
                        }
                        material->SetLightProperties(m_light);
                    }
                }
            }
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
            this->InitScene();
            this->InitUI();
            this->InitRenderTexture();
        }

        virtual void Done()
        {
            if (m_blit_color_camera)
            {
                Display::Instance()->DestroyCamera(m_blit_color_camera);
                m_blit_color_camera = nullptr;
            }
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

            Shader::RemoveCache("Diffuse");
        }

        virtual void Update()
        {
            if (m_label)
            {
                m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
            }
        }
    };
}
