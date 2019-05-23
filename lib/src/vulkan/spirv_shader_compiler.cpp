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

#include "spirv_shader_compiler.h"
#include "memory/Memory.h"
#include "Debug.h"

#if VR_ANDROID
#include "shaderc/shaderc.hpp"

struct shader_type_mapping
{
    Viry3D::ShaderCompiler::ShaderType shader_type;
    shaderc_shader_kind shaderc_type;
};

static const shader_type_mapping shader_map_table[] =
{
    {Viry3D::ShaderCompiler::ShaderType::Vertex, shaderc_glsl_vertex_shader},
    {Viry3D::ShaderCompiler::ShaderType::Fragment, shaderc_glsl_fragment_shader},
};

shaderc_shader_kind MapShadercType(Viry3D::ShaderCompiler::ShaderType shader_type)
{
    for (auto shader : shader_map_table)
    {
        if (shader.shader_type == shader_type)
        {
            return shader.shaderc_type;
        }
    }
    return shaderc_glsl_infer_from_source;
}
#else
#undef max
#include "glslang/SPIRV/GlslangToSpv.h"
#endif

namespace Viry3D
{
	namespace ShaderCompiler
	{
		void InitShaderCompiler()
		{
#if VR_WINDOWS || VR_UWP
			glslang::InitializeProcess();
#endif
		}

		void DeinitShaderCompiler()
		{
#if VR_WINDOWS || VR_UWP
			glslang::FinalizeProcess();
#endif
		}

#if VR_WINDOWS || VR_UWP
		static void InitResources(TBuiltInResource& resources)
		{
			resources.maxLights = 32;
			resources.maxClipPlanes = 6;
			resources.maxTextureUnits = 32;
			resources.maxTextureCoords = 32;
			resources.maxVertexAttribs = 64;
			resources.maxVertexUniformComponents = 4096;
			resources.maxVaryingFloats = 64;
			resources.maxVertexTextureImageUnits = 32;
			resources.maxCombinedTextureImageUnits = 80;
			resources.maxTextureImageUnits = 32;
			resources.maxFragmentUniformComponents = 4096;
			resources.maxDrawBuffers = 32;
			resources.maxVertexUniformVectors = 128;
			resources.maxVaryingVectors = 8;
			resources.maxFragmentUniformVectors = 16;
			resources.maxVertexOutputVectors = 16;
			resources.maxFragmentInputVectors = 15;
			resources.minProgramTexelOffset = -8;
			resources.maxProgramTexelOffset = 7;
			resources.maxClipDistances = 8;
			resources.maxComputeWorkGroupCountX = 65535;
			resources.maxComputeWorkGroupCountY = 65535;
			resources.maxComputeWorkGroupCountZ = 65535;
			resources.maxComputeWorkGroupSizeX = 1024;
			resources.maxComputeWorkGroupSizeY = 1024;
			resources.maxComputeWorkGroupSizeZ = 64;
			resources.maxComputeUniformComponents = 1024;
			resources.maxComputeTextureImageUnits = 16;
			resources.maxComputeImageUniforms = 8;
			resources.maxComputeAtomicCounters = 8;
			resources.maxComputeAtomicCounterBuffers = 1;
			resources.maxVaryingComponents = 60;
			resources.maxVertexOutputComponents = 64;
			resources.maxGeometryInputComponents = 64;
			resources.maxGeometryOutputComponents = 128;
			resources.maxFragmentInputComponents = 128;
			resources.maxImageUnits = 8;
			resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
			resources.maxCombinedShaderOutputResources = 8;
			resources.maxImageSamples = 0;
			resources.maxVertexImageUniforms = 0;
			resources.maxTessControlImageUniforms = 0;
			resources.maxTessEvaluationImageUniforms = 0;
			resources.maxGeometryImageUniforms = 0;
			resources.maxFragmentImageUniforms = 8;
			resources.maxCombinedImageUniforms = 8;
			resources.maxGeometryTextureImageUnits = 16;
			resources.maxGeometryOutputVertices = 256;
			resources.maxGeometryTotalOutputComponents = 1024;
			resources.maxGeometryUniformComponents = 1024;
			resources.maxGeometryVaryingComponents = 64;
			resources.maxTessControlInputComponents = 128;
			resources.maxTessControlOutputComponents = 128;
			resources.maxTessControlTextureImageUnits = 16;
			resources.maxTessControlUniformComponents = 1024;
			resources.maxTessControlTotalOutputComponents = 4096;
			resources.maxTessEvaluationInputComponents = 128;
			resources.maxTessEvaluationOutputComponents = 128;
			resources.maxTessEvaluationTextureImageUnits = 16;
			resources.maxTessEvaluationUniformComponents = 1024;
			resources.maxTessPatchComponents = 120;
			resources.maxPatchVertices = 32;
			resources.maxTessGenLevel = 64;
			resources.maxViewports = 16;
			resources.maxVertexAtomicCounters = 0;
			resources.maxTessControlAtomicCounters = 0;
			resources.maxTessEvaluationAtomicCounters = 0;
			resources.maxGeometryAtomicCounters = 0;
			resources.maxFragmentAtomicCounters = 8;
			resources.maxCombinedAtomicCounters = 8;
			resources.maxAtomicCounterBindings = 1;
			resources.maxVertexAtomicCounterBuffers = 0;
			resources.maxTessControlAtomicCounterBuffers = 0;
			resources.maxTessEvaluationAtomicCounterBuffers = 0;
			resources.maxGeometryAtomicCounterBuffers = 0;
			resources.maxFragmentAtomicCounterBuffers = 1;
			resources.maxCombinedAtomicCounterBuffers = 1;
			resources.maxAtomicCounterBufferSize = 16384;
			resources.maxTransformFeedbackBuffers = 4;
			resources.maxTransformFeedbackInterleavedComponents = 64;
			resources.maxCullDistances = 8;
			resources.maxCombinedClipAndCullDistances = 8;
			resources.maxSamples = 4;
			resources.limits.nonInductiveForLoops = 1;
			resources.limits.whileLoops = 1;
			resources.limits.doWhileLoops = 1;
			resources.limits.generalUniformIndexing = 1;
			resources.limits.generalAttributeMatrixVectorIndexing = 1;
			resources.limits.generalVaryingIndexing = 1;
			resources.limits.generalSamplerIndexing = 1;
			resources.limits.generalVariableIndexing = 1;
			resources.limits.generalConstantMatrixVectorIndexing = 1;
		}

		static EShLanguage FindShaderType(ShaderType shader_type)
		{
			switch (shader_type)
			{
			case ShaderType::Vertex:
				return EShLangVertex;

			case ShaderType::Fragment:
				return EShLangFragment;

			default:
				return EShLangVertex;
			}
		}
#endif

		bool GlslToSpv(ShaderType shader_type, const char* src, Vector<unsigned int>& spirv, String& error)
		{
#if VR_WINDOWS || VR_UWP
			EShLanguage type = FindShaderType(shader_type);
			glslang::TShader shader(type);
			const char *shader_strings[1];
			shader_strings[0] = src;
			shader.setStrings(shader_strings, 1);

			TBuiltInResource resources;
			InitResources(resources);
			EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
			if (!shader.parse(&resources, 100, false, messages))
			{
				error = shader.getInfoLog();
				error += "\n";
				error += shader.getInfoDebugLog();
				return false;
			}

			glslang::TProgram program;
			program.addShader(&shader);

			if (!program.link(messages))
			{
				error = shader.getInfoLog();
				error += "\n";
				error += shader.getInfoDebugLog();
				return false;
			}

			const glslang::TIntermediate* intermediate = program.getIntermediate(type);
			std::vector<unsigned int> spirv_out;
			glslang::GlslangToSpv(*intermediate, spirv_out);

			spirv.Resize((int) spirv_out.size());
			Memory::Copy(&spirv[0], &spirv_out[0], spirv.SizeInBytes());
#else
			shaderc::Compiler compiler;
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(src, strlen(src), MapShadercType(shader_type), "shader");
			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				Log("Error: Id=%d, Msg=%s", module.GetCompilationStatus(), module.GetErrorMessage().c_str());
				return false;
			}

			spirv.Clear();
			for (auto i : module)
			{
				spirv.Add(i);
			}
#endif

			return true;
		}
	}
}
