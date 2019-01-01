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

#include "Display.h"

namespace Viry3D
{
    struct RenderState
    {
#if VR_VULKAN
        enum class Cull
        {
            Off = VK_CULL_MODE_NONE,
            Back = VK_CULL_MODE_BACK_BIT,
            Front = VK_CULL_MODE_FRONT_BIT,
        };

        enum class ZTest
        {
            Off = 0,
            Less = VK_COMPARE_OP_LESS,
            Greater = VK_COMPARE_OP_GREATER,
            LEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
            GEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
            Equal = VK_COMPARE_OP_EQUAL,
            NotEqual = VK_COMPARE_OP_NOT_EQUAL,
            Always = VK_COMPARE_OP_ALWAYS,
        };
#elif VR_GLES
        enum class Cull
        {
            Off = 0,
            Back = GL_BACK,
            Front = GL_FRONT,
        };

        enum class ZTest
        {
            Off = 0,
            Less = GL_LESS,
            Greater = GL_GREATER,
            LEqual = GL_LEQUAL,
            GEqual = GL_GEQUAL,
            Equal = GL_EQUAL,
            NotEqual = GL_NOTEQUAL,
            Always = GL_ALWAYS,
        };
#endif

        enum class ZWrite
        {
            Off = 0,
            On = 1,
        };

        enum class Blend
        {
            Off = 0,
            On = 1,
        };

#if VR_VULKAN
        enum class BlendMode
        {
            Zero = VK_BLEND_FACTOR_ZERO,
            One = VK_BLEND_FACTOR_ONE,
            SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
            SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
            DstColor = VK_BLEND_FACTOR_DST_COLOR,
            DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
            OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
            OneMinusSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
            OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        };
#elif VR_GLES
        enum class BlendMode
        {
            Zero = GL_ZERO,
            One = GL_ONE,
            SrcColor = GL_SRC_COLOR,
            SrcAlpha = GL_SRC_ALPHA,
            DstColor = GL_DST_COLOR,
            DstAlpha = GL_DST_ALPHA,
            OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
            OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
            OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
            OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
        };
#endif

        enum class Queue
        {
            Background = 1000,
            Geometry = 2000,
            AlphaTest = 2450,
            Transparent = 3000,
            Overlay = 4000,
        };

        Cull cull;
        ZTest zTest;
        ZWrite zWrite;
        Blend blend;
        BlendMode srcBlendMode;
        BlendMode dstBlendMode;
        int queue;

        RenderState():
            cull(Cull::Back),
            zTest(ZTest::LEqual),
            zWrite(ZWrite::On),
            blend(Blend::Off),
            srcBlendMode(BlendMode::SrcAlpha),
            dstBlendMode(BlendMode::OneMinusSrcAlpha),
            queue((int) Queue::Geometry)
        {
        }
    };
}
