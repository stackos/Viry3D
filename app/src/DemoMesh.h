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
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
    class DemoMesh : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Quaternion rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 3, -4),
            Quaternion::Euler(30, 0, 0),
            45,
            0.3f,
            1000
        };
        struct LightParam
        {
            Color ambient_color;
            Color light_color;
            float light_intensity;
            Quaternion light_rot;
        };
        LightParam m_light_param = {
            Color(0.2f, 0.2f, 0.2f, 1),
            Color(1, 1, 1, 1),
            0.8f,
            Quaternion::Euler(45, 60, 0)
        };

        Camera* m_camera;
        Camera* m_ui_camera;
        Vector<Ref<MeshRenderer>> m_renderers;
        Label* m_label;

        void InitCamera()
        {
            m_camera = Display::Instance()->CreateCamera();
            m_camera->SetDepth(0);
            m_camera->SetLocalPosition(m_camera_param.pos);
            m_camera->SetLocalRotation(m_camera_param.rot);

            m_camera->SetFieldOfView(m_camera_param.fov);
            m_camera->SetNearClip(m_camera_param.near_clip);
            m_camera->SetFarClip(m_camera_param.far_clip);
        }

        void InitMesh()
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

            // plane
            auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/checkflag.png", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true);

            auto material = RefMake<Material>(shader);
            material->SetTexture("u_texture", texture);
            material->SetVector("u_uv_scale_offset", Vector4(10, 10, 0, 0));
            material->SetColor("u_ambient_color", m_light_param.ambient_color);
            material->SetColor("u_light_color", m_light_param.light_color);
            material->SetVector("u_light_dir", m_light_param.light_rot * Vector3(0, 0, 1));
            material->SetFloat("u_light_intensity", m_light_param.light_intensity);

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
            material->SetColor("u_ambient_color", m_light_param.ambient_color);
            material->SetColor("u_light_color", m_light_param.light_color);
            material->SetVector("u_light_dir", m_light_param.light_rot * Vector3(0, 0, 1));
            material->SetFloat("u_light_intensity", m_light_param.light_intensity);

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
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitMesh();
            this->InitUI();
        }

        virtual void Done()
        {
            m_renderers.Clear();

            Display::Instance()->DestroyCamera(m_ui_camera);
            m_ui_camera = nullptr;
            Display::Instance()->DestroyCamera(m_camera);
            m_camera = nullptr;
        }

        virtual void Update()
        {
            m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
        }
    };
}
