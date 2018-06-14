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

#include "vulkan/vulkan_include.h"
#include "memory/Ref.h"
#include "container/Vector.h"
#include "CameraClearFlags.h"

namespace Viry3D
{
    class Camera;
    class Texture;
    class DisplayPrivate;

    class Display
    {
    public:
        static Display* GetDisplay();
        Display(void* window, int width, int height);
        virtual ~Display();
        void OnResize(int width, int height);
        void OnDraw();
        int GetWidth() const;
        int GetHeight() const;
        VkDevice GetDevice() const;
        Camera* CreateCamera();
        void DestroyCamera(Camera* camera);
        void MarkPrimaryCmdDirty();
        void CreateRenderPass(
            const Ref<Texture>& color_texture,
            const Ref<Texture>& depth_texture,
            CameraClearFlags clear_flag,
            VkRenderPass* render_pass,
            Vector<VkFramebuffer>& framebuffers);

    private:
        DisplayPrivate* m_private;
    };
}
