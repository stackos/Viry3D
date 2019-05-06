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

#include "Shader.h"

#if VR_VULKAN
#if VR_WINDOWS || VR_ANDROID
#include "vulkan/vulkan_shader_compiler.h"
#elif VR_IOS || VR_MAC
#include "GLSLConversion.h"
#endif
#endif

namespace Viry3D
{
#if VR_VULKAN
    static void GlslToSpirv(const String& glsl, VkShaderStageFlagBits shader_type, Vector<unsigned int>& spirv)
    {
#if VR_WINDOWS || VR_ANDROID
        String error;
        bool success = GlslToSpv(shader_type, glsl.CString(), spirv, error);
        if (!success)
        {
            Log("shader compile error: %s", error.CString());
        }
        assert(success);
#elif VR_IOS || VR_MAC
        MVKShaderStage stage;
        switch (shader_type)
        {
            case VK_SHADER_STAGE_COMPUTE_BIT:
                stage = kMVKShaderStageCompute;
                break;
            case VK_SHADER_STAGE_VERTEX_BIT:
                stage = kMVKShaderStageVertex;
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                stage = kMVKShaderStageFragment;
                break;
            default:
                stage = kMVKShaderStageAuto;
                break;
        }
        uint32_t* spirv_code = nullptr;
        size_t size = 0;
        char* log = nullptr;
        bool success = mvkConvertGLSLToSPIRV(glsl.CString(),
                                             stage,
                                             &spirv_code,
                                             &size,
                                             &log,
                                             true,
                                             true);
        if (!success)
        {
            Log("shader compile error: %s", log);
        }
        assert(success);
        
        spirv.Resize((int) size / 4);
        Memory::Copy(&spirv[0], spirv_code, spirv.SizeInBytes());
        
        free(log);
        free(spirv_code);
#endif
    }
#endif
    
    void Shader::Init()
    {
#if VR_VULKAN
#if VR_WINDOWS || VR_ANDROID
        InitShaderCompiler();
#endif
#endif
    }
    
    void Shader::Done()
    {
#if VR_VULKAN
#if VR_WINDOWS || VR_ANDROID
        DeinitShaderCompiler();
#endif
#endif
    }
    
    Shader::Shader(const String& name)
    {
        this->SetName(name);
    }
    
    Shader::~Shader()
    {
        
    }
}
