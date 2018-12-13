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

#include "DemoMesh.h"
#include "graphics/Computer.h"

namespace Viry3D
{
    class DemoComputeStorageImage : public DemoMesh
    {
    public:
        Camera* m_blit_origin_camera = nullptr;
        Camera* m_blit_result_camera = nullptr;

        void InitCompute()
        {
            auto compute_input = Texture::LoadTexture2DFromFile(
                Application::Instance()->GetDataPath() + "/texture/env/prefilter/0_0.png",
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge,
                false,
                true);
            auto compute_output = Texture::CreateStorageTexture2D(
                compute_input->GetWidth(),
                compute_input->GetHeight(),
                TextureFormat::R8G8B8A8,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            
            String cs = R"(#version 310 es
layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform readonly lowp image2D inputImage;
layout (binding = 1, rgba8) uniform writeonly lowp image2D resultImage;

float conv(in float[9] kernel, in float[9] data, in float denom, in float offset) 
{
    float res = 0.0;
    for (int i = 0; i < 9; ++i) 
    {
        res += kernel[i] * data[i];
    }
    return clamp(res / denom + offset, 0.0, 1.0);
}

struct ImageData 
{
    float avg[9];
} imageData;	

void main()
{	
    int n = -1;
    for (int i = -1; i < 2; ++i) 
    {   
	    for (int j = -1; j < 2; ++j) 
	    {    
		    n++;    
		    vec3 rgb = imageLoad(inputImage, ivec2(int(gl_GlobalInvocationID.x) + i, int(gl_GlobalInvocationID.y) + j)).rgb;
		    imageData.avg[n] = (rgb.r + rgb.g + rgb.b) / 3.0;
	    }
    }

    float[9] kernel;
    kernel[0] = -1.0 / 8.0; kernel[1] = -1.0 / 8.0; kernel[2] = -1.0 / 8.0;
    kernel[3] = -1.0 / 8.0; kernel[4] = 1.0;        kernel[5] = -1.0 / 8.0;
    kernel[6] = -1.0 / 8.0; kernel[7] = -1.0 / 8.0; kernel[8] = -1.0 / 8.0;

    vec4 res = vec4(vec3(conv(kernel, imageData.avg, 0.1, 0.0)), 1.0);

    imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res);
}
)";

            auto shader = RefMake<Shader>(cs);
            auto material = RefMake<Material>(shader);
            material->SetTexture("inputImage", compute_input);
            material->SetTexture("resultImage", compute_output);

            auto computer = RefMake<Computer>();
            computer->SetMaterial(material);
            computer->SetWorkgroupCount(compute_input->GetWidth() / 16, compute_input->GetHeight() / 16, 1);

            m_camera->AddRenderer(computer);

            m_blit_origin_camera = Display::Instance()->CreateBlitCamera(1, compute_input, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0, 0, 0.5f, 1));
            m_blit_result_camera = Display::Instance()->CreateBlitCamera(2, compute_output, Ref<Material>(), "", CameraClearFlags::Nothing, Rect(0.5, 0, 0.5f, 1));

            m_ui_camera->SetDepth(3);
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitUI();
            this->InitCompute();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_blit_origin_camera);
            m_blit_origin_camera = nullptr;
            Display::Instance()->DestroyCamera(m_blit_result_camera);
            m_blit_result_camera = nullptr;

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
