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

#if VR_VULKAN
#include "vulkan/vulkan_include.h"
#elif VR_GLES
#include "gles/gles_include.h"
#endif
#include "memory/Ref.h"
#include "container/Vector.h"
#include "CameraClearFlags.h"
#include "string/String.h"
#include "math/Rect.h"
#include "UniformSet.h"
#include "VertexAttribute.h"
#include "Action.h"

namespace Viry3D
{
    class Camera;
    class Texture;
    class Material;
    struct RenderState;
    class BufferObject;
    class DisplayPrivate;

    class Display
    {
    public:
        static Display* Instance();
        Display(const String& name, void* window, int width, int height);
        virtual ~Display();
        void OnResize(int width, int height);
        void OnPause();
		void OnResume();
        void OnDraw();
        int GetWidth() const;
        int GetHeight() const;
        Camera* CreateCamera();
        Camera* CreateBlitCamera(int depth, const Ref<Texture>& texture, const Ref<Material>& material = Ref<Material>(), const String& texture_name = "", CameraClearFlags clear_flags = CameraClearFlags::Invalidate, const Rect& rect = Rect(0, 0, 1, 1));
        void DestroyCamera(Camera* camera);
        int GetMaxSamples();
#if VR_VULKAN
        VkDevice GetDevice() const;
        void WaitDevice() const;
        void MarkPrimaryCmdDirty();
        void CreateRenderPass(
            const Ref<Texture>& color_texture,
            const Ref<Texture>& depth_texture,
            const Vector<Ref<Texture>>& extra_color_textures,
            CameraClearFlags clear_flag,
            bool stereo_rendering,
            VkRenderPass* render_pass,
            Vector<VkFramebuffer>& framebuffers);
        void CreateCommandPool(VkCommandPool* cmd_pool);
        void CreateComputeCommandPool(VkCommandPool* cmd_pool);
        void CreateCommandBuffer(VkCommandPool cmd_pool, VkCommandBufferLevel level, VkCommandBuffer* cmd);
        void CreateShaderModule(
            const String& vs_predefine,
            const Vector<String>& vs_includes,
            const String& vs_source,
            const String& fs_predefine,
            const Vector<String>& fs_includes,
            const String& fs_source,
            VkShaderModule* vs_module,
            VkShaderModule* fs_module,
            Vector<VertexAttribute>& attributes,
            Vector<UniformSet>& uniform_sets);
        void CreateComputeShaderModule(
            const String& cs_source,
            VkShaderModule* cs_module,
            Vector<UniformSet>& uniform_sets);
        void CreatePipelineCache(VkPipelineCache* pipeline_cache);
        void CreatePipelineLayout(
            const Vector<UniformSet>& uniform_sets,
            Vector<VkDescriptorSetLayout>& descriptor_layouts,
            VkPipelineLayout* pipeline_layout);
        void CreatePipeline(
            VkRenderPass render_pass,
            const Vector<VertexAttribute>& attributes,
            VkShaderModule vs_module,
            VkShaderModule fs_module,
            const RenderState& render_state,
            VkPipelineLayout pipeline_layout,
            VkPipelineCache pipeline_cache,
            VkPipeline* pipeline,
            bool color_attachment,
            bool depth_attachment,
            int extra_color_attachment_count,
            int sample_count,
            bool instancing,
            int instance_stride);
        void CreateComputePipeline(
            VkShaderModule cs_module,
            VkPipelineLayout pipeline_layout,
            VkPipelineCache pipeline_cache,
            VkPipeline* pipeline);
        void CreateDescriptorSetPool(const Vector<UniformSet>& uniform_sets, VkDescriptorPool* descriptor_pool);
        void CreateDescriptorSets(
            const Vector<UniformSet>& uniform_sets,
            VkDescriptorPool descriptor_pool,
            const Vector<VkDescriptorSetLayout>& descriptor_layouts,
            Vector<VkDescriptorSet>& descriptor_sets);
        void CreateUniformBuffer(VkDescriptorSet descriptor_set, UniformBuffer& buffer);
        void UpdateUniformTexture(VkDescriptorSet descriptor_set, int binding, bool is_storage, const Ref<Texture>& texture);
        void UpdateStorageBuffer(VkDescriptorSet descriptor_set, int binding, const Ref<BufferObject>& buffer);
        Ref<BufferObject> CreateBuffer(const void* data, int size, VkBufferUsageFlags usage);
        void UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size);
        void ReadBuffer(const Ref<BufferObject>& buffer, ByteBuffer& data);
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
            const Ref<BufferObject>& draw_buffer,
            const Ref<BufferObject>& instance_buffer);
        void BuildComputeInstanceCmd(
            VkCommandBuffer cmd,
            VkPipelineLayout pipeline_layout,
            VkPipeline pipeline,
            const Vector<VkDescriptorSet>& descriptor_sets,
            const Ref<BufferObject>& dispatch_buffer);
		void BuildEmptyInstanceCmd(VkCommandBuffer cmd, VkRenderPass render_pass);
        void BuildEmptyComputeInstanceCmd(VkCommandBuffer cmd);
        VkFormat ChooseFormatSupported(const Vector<VkFormat>& formats, VkFormatFeatureFlags features);
        Ref<Texture> CreateTexture(
            VkImageType type,
            VkImageViewType view_type,
            int width,
            int height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspect_flag,
            const VkComponentMapping& component,
            int mipmap_level_count,
            bool cubemap,
            int array_size,
            int sample_count);
        void CreateSampler(
            const Ref<Texture>& texture,
            VkFilter filter_mode,
            VkSamplerAddressMode wrap_mode);
        void BeginImageCmd();
        void EndImageCmd();
        void SetImageLayout(
            VkImage image,
			VkPipelineStageFlags src_stage,
			VkPipelineStageFlags dst_stage,
            const VkImageSubresourceRange& subresource_range,
            VkImageLayout old_image_layout,
            VkImageLayout new_image_layout,
            VkAccessFlagBits src_access_mask);
        VkCommandBuffer GetImageCmd() const;
        bool IsSupportMultiview() const;
#elif VR_GLES
        void EnableGLESv3();
        bool IsGLESv3() const;
        Ref<BufferObject> CreateBuffer(const void* data, int size, GLenum target, GLenum usage);
        void UpdateBuffer(const Ref<BufferObject>& buffer, int buffer_offset, const void* data, int size);
        void BindSharedContext() const;
        void UnbindSharedContext() const;
        void Flush() const;
#if VR_IOS
        void SetBindDefaultFramebufferImplemment(Action action);
        void BindDefaultFramebuffer() const;
#elif VR_WASM
        enum class Platform
        {
            Android,
            iOS,
            Mac,
            Windows,
            Other,
        };
        void SetPlatform(Platform platform);
        Platform GetPlatform() const;
#endif
#endif

    private:
        DisplayPrivate* m_private;
    };
}
