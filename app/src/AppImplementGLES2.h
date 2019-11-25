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

#include "AppInclude.h"

namespace Viry3D
{
    class AppImplementGLES2 : public AppImplement
    {
    public:
        Label* m_fps_label = nullptr;

        AppImplementGLES2()
        {
            auto camera = GameObject::Create("")->AddComponent<Camera>();
            camera->GetTransform()->SetPosition(Vector3(0, 0, -5));
            camera->SetDepth(0);
            camera->SetClearColor(Color(0, 0, 1, 1));
            camera->SetCullingMask(1 << 0);

            this->InitKTXTest();
            this->InitGPUBlendShapeTest();
            this->InitUI();
        }

        void InitKTXTest()
        {
            auto texture = Texture::LoadFromKTXFile(
#if VR_IOS
                Engine::Instance()->GetDataPath() + "/texture/ktx/checkflag_PVRTC_RGB_4V1.KTX",
#elif VR_ANDROID
                Engine::Instance()->GetDataPath() + "/texture/ktx/logo_JPG_ETC_RGB.KTX",
#else
                Engine::Instance()->GetDataPath() + "/texture/ktx/logo_JPG_BC1_RGB.KTX",
#endif
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);

            auto material = this->InitVertexTextureTest();
            material->SetTexture(MaterialProperty::TEXTURE, texture);

            auto renderer = GameObject::Create("")->AddComponent<MeshRenderer>();
            renderer->GetGameObject()->SetLayer(0);
            renderer->SetMesh(Resources::LoadMesh("Library/unity default resources.Cube.mesh"));
            renderer->SetMaterial(material);
        }

        uint32_t FloatToBin(float f)
        {
            uint32_t b = 0;
            int s = 0;
            int e = 0;
            float fp = 0;

            if (f < 0)
            {
                s = 1;
                fp = -f;
            }
            else
            {
                s = 0;
                fp = f;
            }

            int i = (int) floor(fp);
            float d = fp - i;

            std::list<char> ib;
            do
            {
                ib.push_front(i % 2);
                i /= 2;
            } while (i > 0);

            std::list<char> db;
            do
            {
                d *= 2;
                db.push_back((char) floor(d));
                d -= floor(d);
            } while (d > 0 && db.size() < 23 + 127);

            int move_left = 0;
            if (ib.size() == 1)
            {
                if (ib.front() == 0)
                {
                    while (db.size() > 0 && db.front() == 0 && move_left > -126)
                    {
                        --move_left;
                        db.pop_front();
                    }
                    if (db.size() > 0 && move_left > -126)
                    {
                        --move_left;
                        db.pop_front();
                    }
                    else
                    {
                        move_left = -127;
                    }
                }
            }
            else
            {
                while (ib.size() > 1)
                {
                    ++move_left;
                    db.push_front(ib.back());
                    ib.pop_back();
                }
            }

            e = move_left + 127;
            assert(e >= 0);

            while (db.size() < 23)
            {
                db.push_back(0);
            }
            while (db.size() > 23)
            {
                db.pop_back();
            }

            b |= s << 31;
            b |= e << 23;

            int move = 22;
            for (auto i : db)
            {
                b |= i << move;
                --move;
            }

            int* fb = (int*) &f;

            assert(*fb == b);

            return b;
        }

        uint16_t FLoatToHalf(float f)
        {
            uint16_t h = 0;
            uint32_t fb = *(uint32_t*) &f;
            int s = (fb >> 31) & 0x1;
            int e = (fb >> 23) & 0xff;
            if (e == 0)
            {
                e = 0;
            }
            else if (e == 255)
            {
                assert(false);
            }
            else
            {
                e -= (1 << (8 - 1)) - 1;
                e += (1 << (5 - 1)) - 1;
                assert(e > 0 && e < 31);
            }
            int m = (fb >> 0) & 0x7fffff;
            m >>= (23 - 10);

            h |= s << 15;
            h |= e << 10;
            h |= m;

            return h;
        }

        Ref<Material> InitVertexTextureTest()
        {
            // create shader test using vertex float texture
            Shader::Pass pass;
            pass.pipeline.rasterState.depthWrite = true;
            pass.pipeline.rasterState.colorWrite = true;

            if (Engine::Instance()->GetShaderModel() == filament::backend::ShaderModel::GL_ES_20)
            {
                pass.vs = R"(
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;
uniform sampler2D u_vertex_texture;
attribute vec4 i_vertex;
attribute vec2 i_uv;
varying vec2 v_uv;
varying vec4 v_color;
void main()
{
    mat4 model_matrix = u_model_matrix;
	gl_Position = vec4(i_vertex.xyz, 1.0) * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv;
    v_color = texture2D(u_vertex_texture, v_uv);
	vk_convert();
}
)";
                pass.fs = R"(
precision highp float;
uniform sampler2D u_texture;
varying vec2 v_uv;
varying vec4 v_color;
void main()
{
	vec4 c = texture2D(u_texture, v_uv) * v_color;
	gl_FragColor = c;
}
)";
            }
            else
            {
                pass.vs = R"(
VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
};
VK_UNIFORM_BINDING(1) uniform PerRenderer
{
	mat4 u_model_matrix;
};
VK_SAMPLER_BINDING(0) uniform sampler2D u_vertex_texture;
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;
VK_LAYOUT_LOCATION(1) out vec4 v_color;
void main()
{
    mat4 model_matrix = u_model_matrix;
	gl_Position = vec4(i_vertex.xyz, 1.0) * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv;
    v_color = texture(u_vertex_texture, v_uv);
	vk_convert();
}
)";
                pass.fs = R"(
precision highp float;
VK_SAMPLER_BINDING(1) uniform sampler2D u_texture;
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
VK_LAYOUT_LOCATION(1) in vec4 v_color;
layout(location = 0) out vec4 o_color;
void main()
{
	vec4 c = texture(u_texture, v_uv) * v_color;
	o_color = c;
}
)";
            }

            Shader::Uniform u;
            u.name = "PerView";
            u.binding = (int) Shader::BindingPoint::PerView;
            u.members.Add({ "u_view_matrix", 0, 64 });
            u.members.Add({ "u_projection_matrix", 64, 64 });
            u.size = 128;
            pass.uniforms.Add(u);
            u.name = "PerRenderer";
            u.binding = (int) Shader::BindingPoint::PerRenderer;
            u.members.Add({ "u_model_matrix", 0, 64 });
            u.size = 64;
            pass.uniforms.Add(u);

            Shader::SamplerGroup s;
            s.name = "PerMaterialVertex";
            s.binding = (int) Shader::BindingPoint::PerMaterialVertex;
            s.samplers.Add({ "u_vertex_texture", 0 });
            pass.samplers.Add(s);
            s = {};
            s.name = "PerMaterialFragment";
            s.binding = (int) Shader::BindingPoint::PerMaterialFragment;
            s.samplers.Add({ "u_texture", 1 });
            pass.samplers.Add(s);

            Vector<Shader::Pass> passes({ pass });
            auto material = RefMake<Material>(Shader::Create(passes));

#if 0
            if (Texture::SelectFormat({ TextureFormat::R16F }, false) == TextureFormat::None)
            {
                material = RefMake<Material>(Shader::Find("Unlit/Texture"));
            }
            else
            {
                ByteBuffer pixels(2 * 2 * 2);
                uint16_t* p = (uint16_t*) pixels.Bytes();
                p[0] = FLoatToHalf(0); p[1] = FLoatToHalf(0); p[2] = FLoatToHalf(1); p[3] = FLoatToHalf(1);
                auto texture = Texture::CreateTexture2DFromMemory(
                    pixels,
                    2,
                    2,
                    TextureFormat::R16F,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge,
                    false);

                material->SetTexture("u_vertex_texture", texture);
            }
#else
            if (Texture::SelectFormat({ TextureFormat::R16G16B16A16F }, false) == TextureFormat::None)
            {
                material = RefMake<Material>(Shader::Find("Unlit/Texture"));
            }
            else
            {
                ByteBuffer pixels(2 * 2 * 2 * 4);
                uint16_t* p = (uint16_t*) pixels.Bytes();
                p[0] = FLoatToHalf(0); p[1] = FLoatToHalf(0); p[2] = FLoatToHalf(1); p[3] = FLoatToHalf(1);
                p[4] = FLoatToHalf(0); p[5] = FLoatToHalf(1); p[6] = FLoatToHalf(0); p[7] = FLoatToHalf(1);
                p[8] = FLoatToHalf(1); p[9] = FLoatToHalf(0); p[10] = FLoatToHalf(0); p[11] = FLoatToHalf(1);
                p[12] = FLoatToHalf(0); p[13] = FLoatToHalf(0); p[14] = FLoatToHalf(0); p[15] = FLoatToHalf(1);
                auto texture = Texture::CreateTexture2DFromMemory(
                    pixels,
                    2,
                    2,
                    TextureFormat::R16G16B16A16F,
                    FilterMode::Nearest,
                    SamplerAddressMode::ClampToEdge,
                    false);

                material->SetTexture("u_vertex_texture", texture);
            }
#endif

            // test float to binary
            FloatToBin(0.000000000000000000000000000000000000000000001f);
            FloatToBin(0);
            FloatToBin(-0);
            FloatToBin(3490593);
            FloatToBin(0.5f);
            FloatToBin(20.59375f);
            FloatToBin(-12.5);
            FloatToBin(2.025675f);
            FloatToBin(-0.046875f);
            // FloatToBin(3.39999995e+38f); // failed

            return material;
        }

        void InitGPUBlendShapeTest()
        {
            auto model = Resources::LoadGameObject("Resources/res/model/unitychan/unitychan.go");
            model->GetTransform()->SetPosition(Vector3(1.4f, -1, -2));
            model->GetTransform()->SetRotation(Quaternion::Euler(0, 200, 0));
            auto anim = model->GetComponent<Animation>();
            anim->Play(0);
        }

        void InitUI()
        {
            auto ui_camera = GameObject::Create("")->AddComponent<Camera>();
            ui_camera->SetClearFlags(CameraClearFlags::Nothing);
            ui_camera->SetDepth(1);
            ui_camera->SetCullingMask(1 << 1);

            auto canvas = GameObject::Create("")->AddComponent<CanvasRenderer>(FilterMode::Linear);
            canvas->GetGameObject()->SetLayer(1);
            canvas->SetCamera(ui_camera);

            auto label = RefMake<Label>();
            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetOffset(Vector2i(0, 0));
            label->SetColor(Color(1, 1, 1, 1));
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
            canvas->AddView(label);
            m_fps_label = label.get();
            m_fps_label->SetText(String::Format("FPS:%d DC:%d", Time::GetFPS(), Time::GetDrawCall()));

            this->InitMultiAtlasTest(canvas);
        }

        void InitMultiAtlasTest(const Ref<CanvasRenderer>& canvas)
        {
            for (int i = 0; i < 4; ++i)
            {
                auto sprite = RefMake<Sprite>();
                sprite->SetAlignment(ViewAlignment::Left | ViewAlignment::VCenter);
                sprite->SetPivot(Vector2(0, 0.5f));
                sprite->SetOffset(Vector2i(i * 220, 220));
                sprite->SetSize(Vector2i(200, 200));
                canvas->AddView(sprite);

                Resources::LoadFileFromUrlAsync("texture/logo.jpg", [=](const ByteBuffer& buffer) {
                    Log("buffer size %d", buffer.Size());
                    auto texture = Texture::LoadTexture2DFromMemory(buffer, FilterMode::Linear, SamplerAddressMode::ClampToEdge, false);
                    Log("texture width:%d height:%d", texture->GetWidth(), texture->GetHeight());
                    sprite->SetTexture(texture);
                    });
            }
        }

        void Update()
        {
            if (m_fps_label)
            {
                m_fps_label->SetText(String::Format("FPS:%d DC:%d", Time::GetFPS(), Time::GetDrawCall()));
            }
        }
    };
}
