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
#include "Resources.h"

namespace Viry3D
{
    class DemoMesh : public Demo
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
            Vector3(0, 3, -4),
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
        Vector<Ref<MeshRenderer>> m_renderers;
        Label* m_label = nullptr;
        Ref<Light> m_light;

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
            RenderState render_state;

#if VR_VULKAN
            auto shader = RefMake<Shader>(
                "",
                Vector<String>({ "Diffuse.vs" }),
                "",
                "",
                Vector<String>({ "Diffuse.fs" }),
                "",
                render_state);
#elif VR_GLES
            auto shader = RefMake<Shader>(
                "",
                Vector<String>({ "Diffuse.100.vs" }),
                "",
                "",
                Vector<String>({ "Diffuse.100.fs" }),
                "",
                render_state);
#endif

            // plane
            auto texture = Resources::LoadTexture("texture/checkflag.png.tex");

            auto material = RefMake<Material>(shader);
            material->SetTexture("u_texture", texture);
            material->SetVector("u_uv_scale_offset", Vector4(10, 10, 0, 0));
            material->SetLightProperties(m_light);

            auto plane = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Plane.mesh");

            auto renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(plane);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            // cube
            auto cube = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cube.mesh");

            material = RefMake<Material>(shader);
            material->SetTexture("u_texture", Texture::GetSharedWhiteTexture());
            material->SetVector("u_uv_scale_offset", Vector4(1, 1, 0, 0));
            material->SetLightProperties(m_light);

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cube);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            renderer->SetLocalPosition(Vector3(-0.7f, 0.72f, 0.8f));
            renderer->SetLocalScale(Vector3(1, 1.44f, 1));

            // sphere
            auto sphere = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Sphere.mesh");

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(sphere);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            renderer->SetLocalPosition(Vector3(0.6f, 0.5f, 0.8f));

            // cylinder
            auto cylinder = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cylinder.mesh");

            renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cylinder);
            m_camera->AddRenderer(renderer);
            m_renderers.Add(renderer);

            renderer->SetLocalPosition(Vector3(-1.4f, 1.2f, 0));
            renderer->SetLocalScale(Vector3(0.2f, 1.2f, 0.2f));
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
            m_renderers.Clear();

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
