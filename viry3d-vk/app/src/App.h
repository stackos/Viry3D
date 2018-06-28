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

#include "Application.h"
#include "Input.h"
#include "Debug.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/VertexAttribute.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "memory/Memory.h"
#include "thread/ThreadPool.h"

using namespace Viry3D;

// TODO:
// - camera view projection matrix build
// - PostProcess
// - CanvaRenderer View Sprite Label Button
// - ScrollView TabView TreeView
// - android project
// - mac project
// - ios project

class App : public Application
{
public:
    Camera* m_camera;
    MeshRenderer* m_renderer;
    float m_deg = 0;

    App()
    {
        auto color_texture = Texture::CreateRenderTexture(
            1280,
            720,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        auto depth_texture = Texture::CreateRenderTexture(
            1280,
            720,
            VK_FORMAT_D32_SFLOAT,
            VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        m_camera = Display::Instance()->CreateCamera();
        m_camera->SetRenderTarget(color_texture, depth_texture);
        m_camera->SetDepth(0);

        auto blit_depth_camera = Display::Instance()->CreateBlitCamera(1, m_camera->GetRenderTargetDepth(), "", CameraClearFlags::Nothing, Ref<Shader>(), Rect(0, 0, 0.25f, 0.25f));
        blit_depth_camera->SetRenderTarget(color_texture, Ref<Texture>());

        Display::Instance()->CreateBlitCamera(2, m_camera->GetRenderTargetColor());

        String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_projection_matrix;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec4 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_projection_matrix;
	v_uv = a_uv;

	vulkan_convert();
}
)";
        String fs = R"(
precision mediump float;
      
UniformTexture(0, 1) uniform sampler2D u_texture;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    o_frag = texture(u_texture, v_uv);
}
)";
        RenderState render_state;

        auto shader = RefMake<Shader>(
            vs,
            Vector<String>(),
            fs,
            Vector<String>(),
            render_state);
        auto material = RefMake<Material>(shader);

        Vector<Vertex> vertices(8);
        Memory::Zero(&vertices[0], vertices.SizeInBytes());
        vertices[0].vertex = Vector3(-0.5f, 0.5f, -0.5f);
        vertices[1].vertex = Vector3(-0.5f, -0.5f, -0.5f);
        vertices[2].vertex = Vector3(0.5f, -0.5f, -0.5f);
        vertices[3].vertex = Vector3(0.5f, 0.5f, -0.5f);
        vertices[4].vertex = Vector3(-0.5f, 0.5f, 0.5f);
        vertices[5].vertex = Vector3(-0.5f, -0.5f, 0.5f);
        vertices[6].vertex = Vector3(0.5f, -0.5f, 0.5f);
        vertices[7].vertex = Vector3(0.5f, 0.5f, 0.5f);
        vertices[0].uv = Vector2(0, 0);
        vertices[1].uv = Vector2(0, 1);
        vertices[2].uv = Vector2(1, 1);
        vertices[3].uv = Vector2(1, 0);
        vertices[4].uv = Vector2(1, 0);
        vertices[5].uv = Vector2(1, 1);
        vertices[6].uv = Vector2(0, 1);
        vertices[7].uv = Vector2(0, 0);

        Vector<unsigned short> indices({
            0, 1, 2, 0, 2, 3,
            3, 2, 6, 3, 6, 7,
            7, 6, 5, 7, 5, 4,
            4, 5, 1, 4, 1, 0,
            4, 0, 3, 4, 3, 7,
            1, 5, 6, 1, 6, 2
            });
        auto mesh = RefMake<Mesh>(vertices, indices);

        auto renderer = RefMake<MeshRenderer>();
        renderer->SetMaterial(material);
        renderer->SetMesh(mesh);
        m_renderer = renderer.get();

        m_camera->AddRenderer(renderer);

        auto texture = Texture::LoadTexture2DFromFile(Application::Instance()->GetDataPath() + "/texture/logo.jpg", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true);
        material->SetTexture("u_texture", texture);

        Vector3 camera_pos(0, 0, -5);
        Matrix4x4 view = Matrix4x4::LookTo(camera_pos, Vector3(0, 0, 1), Vector3(0, 1, 0));
        Matrix4x4 projection = Matrix4x4::Perspective(45, m_camera->GetTargetWidth() / (float) m_camera->GetTargetHeight(), 1, 1000);
        Matrix4x4 view_projection = projection * view;
        material->SetMatrix("u_view_projection_matrix", view_projection);

        // sky box
        vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_projection_matrix;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec4 a_pos;

Output(0) vec3 v_uv;

void main()
{
	gl_Position = (a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_projection_matrix).xyww;
	v_uv = a_pos.xyz;

	vulkan_convert();
}
)";
        fs = R"(
precision mediump float;
      
UniformTexture(0, 1) uniform samplerCube u_texture;

Input(0) vec3 v_uv;

Output(0) vec4 o_frag;

void main()
{
    vec4 c = textureLod(u_texture, v_uv, 0.0);
    o_frag = pow(c, vec4(1.0 / 2.2));
}
)";
        render_state = RenderState();
        render_state.cull = RenderState::Cull::Front;
        render_state.zWrite = RenderState::ZWrite::Off;
        render_state.queue = (int) RenderState::Queue::Background;
        shader = RefMake<Shader>(
            vs,
            Vector<String>(),
            fs,
            Vector<String>(),
            render_state);
        material = RefMake<Material>(shader);

        renderer = RefMake<MeshRenderer>();
        renderer->SetMaterial(material);
        renderer->SetMesh(mesh);

        material->SetMatrix("u_view_projection_matrix", view_projection);

        Matrix4x4 model = Matrix4x4::Translation(camera_pos);
        renderer->SetInstanceMatrix("u_model_matrix", model);

        Thread::Task task;
        task.job = []() {
            auto cubemap = Texture::CreateCubemap(1024, VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true);
            cubemap->UpdateCubemapFaceBegin();
            for (int i = 0; i < cubemap->GetMipmapLevelCount(); ++i)
            {
                for (int j = 0; j < 6; ++j)
                {
                    int width;
                    int height;
                    int bpp;
                    ByteBuffer pixels = Texture::LoadImageFromFile(String::Format((Application::Instance()->GetDataPath() + "/texture/prefilter/%d_%d.png").CString(), i, j), width, height, bpp);
                    cubemap->UpdateCubemapFace(pixels, (CubemapFace) j, i);
                }
            }
            cubemap->UpdateCubemapFaceEnd();
            return cubemap;
        };
        task.complete = [=](const Ref<Thread::Res>& res) {
            material->SetTexture("u_texture", RefCast<Texture>(res));
            m_camera->AddRenderer(renderer);
        };
        Application::Instance()->GetThreadPool()->AddTask(task);
    }

    virtual ~App()
    {
        Display::Instance()->DestroyCamera(m_camera);
        m_camera = nullptr;
    }

    virtual void Update()
    {
        m_deg += 0.1f;

        Matrix4x4 model = Matrix4x4::Rotation(Quaternion::Euler(Vector3(0, m_deg, 0)));
        m_renderer->SetInstanceMatrix("u_model_matrix", model);
    }
};
