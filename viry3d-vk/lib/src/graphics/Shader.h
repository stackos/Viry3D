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

#include "Display.h"
#include "string/String.h"

namespace Viry3D
{
    struct RenderState
    {
        enum class Cull
        {
            Off = 0,
            Back = VK_CULL_MODE_BACK_BIT,
            Front = VK_CULL_MODE_FRONT_BIT,
        };

        enum class ZTest
        {
            Never = VK_COMPARE_OP_NEVER,
            Less = VK_COMPARE_OP_LESS,
            Greater = VK_COMPARE_OP_GREATER,
            LEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
            GEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
            Equal = VK_COMPARE_OP_EQUAL,
            NotEqual = VK_COMPARE_OP_NOT_EQUAL,
            Always = VK_COMPARE_OP_ALWAYS,
        };

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

    class Shader
    {
    public:
        Shader(const String& vertex_shader, const String& fragment_shader, const RenderState& render_state);
        ~Shader();
        const RenderState& GetRenderState() const { return m_render_state; }

    private:
        RenderState m_render_state;
    };
}
