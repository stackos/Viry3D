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
#include "string/String.h"
#include "math/Rect.h"
#include "UniformSet.h"

namespace Viry3D
{
    class Camera;
    class Texture;
    struct RenderState;
    struct BufferObject;
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
        void WaitDevice() const;
        Camera* CreateCamera();
        void DestroyCamera(Camera* camera);
        void MarkPrimaryCmdDirty();
        void CreateRenderPass(
            const Ref<Texture>& color_texture,
            const Ref<Texture>& depth_texture,
            CameraClearFlags clear_flag,
            VkRenderPass* render_pass,
            Vector<VkFramebuffer>& framebuffers);
        void CreateCommandPool(VkCommandPool* cmd_pool);
        void CreateCommandBuffer(VkCommandPool cmd_pool, VkCommandBufferLevel level, VkCommandBuffer* cmd);
        void CreateShaderModule(
            const String& vs_source,
            const Vector<String>& vs_includes,
            const String& fs_source,
            const Vector<String>& fs_includes,
            VkShaderModule* vs_module,
            VkShaderModule* fs_module,
            Vector<UniformSet>& uniform_sets);
        void CreatePipelineCache(VkPipelineCache* pipeline_cache);
        void CreatePipelineLayout(
            const Vector<UniformSet>& uniform_sets,
            Vector<VkDescriptorSetLayout>& descriptor_layouts,
            VkPipelineLayout* pipeline_layout);
        void CreatePipeline(
            VkRenderPass render_pass,
            VkShaderModule vs_module,
            VkShaderModule fs_module,
            const RenderState& render_state,
            VkPipelineLayout pipeline_layout,
            VkPipelineCache pipeline_cache,
            VkPipeline* pipeline);
        void CreateDescriptorSetPool(const Vector<UniformSet>& uniform_sets, VkDescriptorPool* descriptor_pool);
        void CreateDescriptorSets(
            const Vector<UniformSet>& uniform_sets,
            VkDescriptorPool descriptor_pool,
            const Vector<VkDescriptorSetLayout>& descriptor_layouts,
            Vector<VkDescriptorSet>& descriptor_sets,
            Vector<UniformSet>& uniform_sets_out);
        Ref<BufferObject> CreateBuffer(const void* data, int size, VkBufferUsageFlags usage);
        void UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size);
        void BuildInstanceCmd(
            VkCommandBuffer cmd,
            VkRenderPass render_pass,
            VkPipelineLayout pipeline_layout,
            VkPipeline pipeline,
            const Vector<VkDescriptorSet>& descriptor_sets,
            int image_width,
            int image_height,
            const Rect& view_rect,
            const Ref<BufferObject>& vertex_buffer,
            const Ref<BufferObject>& index_buffer,
            int index_offset,
            int index_count);

    private:
        DisplayPrivate* m_private;
    };
}
