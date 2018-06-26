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

#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"
#include "graphics/VertexAttribute.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "memory/Memory.h"

using namespace Viry3D;

// TODO:
// - sprite renderer

class App
{
public:
    Camera* m_camera;
    MeshRenderer* m_renderer;
    float m_deg = 0;

    App()
    {
        m_camera = Display::GetDisplay()->CreateCamera();

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
        
        auto renderer = RefMake<MeshRenderer>();
        m_renderer = renderer.get();

        RenderState render_state;
        auto shader = RefMake<Shader>(
            vs,
            Vector<String>(),
            fs,
            Vector<String>(),
            render_state);
        auto material = RefMake<Material>(shader);
        renderer->SetMaterial(material);

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
        renderer->SetMesh(mesh);

        m_camera->AddRenderer(renderer);

        auto texture = Texture::LoadTexture2DFromFile(Application::DataPath() + "/texture/logo.jpg", VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true);
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

        auto cubemap = Texture::CreateCubemap(1024, VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true);
        cubemap->UpdateCubemapFaceBegin();
        for (int i = 0; i < cubemap->GetMipmapLevelCount(); ++i)
        {
            for (int j = 0; j < 6; ++j)
            {
                int width;
                int height;
                int bpp;
                ByteBuffer pixels = Texture::LoadImageFromFile(String::Format((Application::DataPath() + "/texture/prefilter/%d_%d.png").CString(), i, j), width, height, bpp);
                cubemap->UpdateCubemapFace(pixels, (CubemapFace) j, i);
            }
        }
        cubemap->UpdateCubemapFaceEnd();

        material->SetTexture("u_texture", cubemap);
        material->SetMatrix("u_view_projection_matrix", view_projection);

        renderer = RefMake<MeshRenderer>();
        renderer->SetMaterial(material);
        renderer->SetMesh(mesh);
        m_camera->AddRenderer(renderer);

        Matrix4x4 model = Matrix4x4::Translation(camera_pos);
        renderer->SetInstanceMatrix("u_model_matrix", model);
    }

    ~App()
    {
        Display::GetDisplay()->DestroyCamera(m_camera);
        m_camera = nullptr;
    }

    void Update()
    {
        m_deg += 0.1f;

        Matrix4x4 model = Matrix4x4::Rotation(Quaternion::Euler(Vector3(0, m_deg, 0)));
        m_renderer->SetInstanceMatrix("u_model_matrix", model);
    }
};


#if VR_WINDOWS
#include <Windows.h>

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                int width = lParam & 0xffff;
                int height = (lParam & 0xffff0000) >> 16;

                if (Display::GetDisplay())
                {
                    Display::GetDisplay()->OnResize(width, height);
                }
            }
            break;

        default:
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Application::SetName("viry3d-vk-demo");
    int width = 1280;
    int height = 720;

    String name = Application::Name();

    WNDCLASSEX win_class;
    ZeroMemory(&win_class, sizeof(win_class));

    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WindowProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = hInstance;
    win_class.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = nullptr;
    win_class.lpszClassName = name.CString();
    win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    win_class.hIcon = (HICON) LoadImage(nullptr, "icon.ico", IMAGE_ICON, SM_CXICON, SM_CYICON, LR_LOADFROMFILE);
    win_class.hIconSm = win_class.hIcon;

    if (!RegisterClassEx(&win_class))
    {
        return 0;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD style_ex = 0;

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, style, FALSE);

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2 + wr.left;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2 + wr.top;

    HWND hwnd = CreateWindowEx(
        style_ex,			// window ex style
        name.CString(),		// class name
        name.CString(),		// app name
        style,			    // window style
        x, y,				// x, y
        wr.right - wr.left, // width
        wr.bottom - wr.top, // height
        nullptr,		    // handle to parent
        nullptr,            // handle to menu
        hInstance,			// hInstance
        nullptr);           // no extra parameters
    if (!hwnd)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);

    Display* display = new Display(hwnd, width, height);

    Ref<App> app = RefMake<App>();

    bool exit = false;
    MSG msg;

    while (true)
    {
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                exit = true;
                break;
            }
            else
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        if (exit)
        {
            break;
        }

        app->Update();

        display->OnDraw();
    }

    app.reset();

    delete display;

    return 0;
}
#endif
