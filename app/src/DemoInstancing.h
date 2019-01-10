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
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
    class DemoInstancing: public Demo
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
            Vector3(9 * 1.2f / 2, 9 * 1.2f / 2, -15),
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
        Ref<Material> m_material;
        Color m_color = Color(1, 1, 1, 1);

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
            m_light->SetLocalRotation(Quaternion::Euler(45, 45, 0));
            m_light->SetAmbientColor(m_light_param.ambient_color);
            m_light->SetColor(m_light_param.light_color);
            m_light->SetIntensity(m_light_param.light_intensity);
        }

        void InitMesh()
        {
            auto env = Resources::LoadTexture("texture/env/env.jpg.tex");

            RenderState render_state;

            auto shader = RefMake<Shader>(
                "#define INSTANCING 1",
                Vector<String>({ "PBR.vs" }),
                "",
                "#define INSTANCING 1",
                Vector<String>({ "PBR.fs" }),
                "",
                render_state);

            auto material = RefMake<Material>(shader);
            m_material = material;

            material->SetColor("u_color", m_color);
            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
            material->SetVector("u_uv_scale_offset", Vector4(1, 1, 0, 0));
            material->SetTexture("u_normal", Texture::GetSharedNormalTexture());
            material->SetTexture("u_metallic_smoothness", Texture::GetSharedWhiteTexture());
            material->SetFloat("u_metallic", 1.0f);
            material->SetFloat("u_smoothness", 1.0f);
            material->SetTexture("u_occlusion", Texture::GetSharedWhiteTexture());
            material->SetFloat("u_occlusion_strength", 1.0f);
            material->SetTexture("u_emission", Texture::GetSharedBlackTexture());
            material->SetColor("u_emission_color", Color(0, 0, 0, 0));
            material->SetTexture("u_environment", env);
            material->SetLightProperties(m_light);

            // sphere
            auto sphere = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Sphere.mesh");

            auto renderer = RefMake<MeshRenderer>();
            m_camera->AddRenderer(renderer);

            renderer->SetMaterial(material);
            renderer->SetMesh(sphere);
            renderer->SetLocalPosition(Vector3(0, 0, 0));

            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < 10; ++j)
                {
                    if (i == 0 && j == 0)
                    {
                        continue;
                    }
                    else
                    {
                        renderer->AddInstance(Vector3(i * 1.2f, j * 1.2f, 0), Quaternion::Identity(), Vector3(1, 1, 1));
                    }
                }
            }

            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < 10; ++j)
                {
                    renderer->SetInstanceExtraVector(i * 10 + j, 0, Vector4(i / 9.0f, j / 9.0f, 1, 1));
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

            Vector<String> names = { "R", "G", "B" };

            // sliders
            for (int i = 0; i < names.Size(); ++i)
            {
                label = RefMake<Label>();
                canvas->AddView(label);
                int y = 120 + i * 65;

                label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetPivot(Vector2(0, 0.5f));
                label->SetSize(Vector2i(100, 30));
                label->SetOffset(Vector2i(40, y));
                label->SetFont(Font::GetFont(FontType::Consola));
                label->SetFontSize(28);
                label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetText(names[i]);

                auto slider = RefMake<Slider>();
                canvas->AddView(slider);

                slider->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                slider->SetPivot(Vector2(0, 0.5f));
                slider->SetSize(Vector2i(200, 30));
                slider->SetOffset(Vector2i(80, y));

                label = RefMake<Label>();
                canvas->AddView(label);

                label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
                label->SetPivot(Vector2(0, 0.5f));
                label->SetSize(Vector2i(100, 30));
                label->SetOffset(Vector2i(300, y));
                label->SetFont(Font::GetFont(FontType::Consola));
                label->SetFontSize(28);
                label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

                slider->SetProgress(1.0f);
                slider->SetValueType(Slider::ValueType::Float, Slider::Value(0.0f), Slider::Value(1.0f));
                slider->SetOnValueChange([=](const Slider::Value& value) {
                    switch (i)
                    {
                    case 0:
                        m_color.r = value.float_value;
                        break;
                    case 1:
                        m_color.g = value.float_value;
                        break;
                    case 2:
                        m_color.b = value.float_value;
                        break;
                    }
                    m_material->SetColor("u_color", m_color);
                    label->SetText(String::Format("%.2f", value.float_value));
                });
                label->SetText("1.00");
            }
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
        }
    };
}
