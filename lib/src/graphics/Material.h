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

#include "Object.h"
#include "Display.h"
#include "Color.h"
#include "container/List.h"
#include "container/Map.h"
#include "math/Matrix4x4.h"
#include "math/Vector4.h"
#include "string/String.h"
#include "memory/Memory.h"

#define MODEL_MATRIX "u_model_matrix"
#define VIEW_MATRIX "u_view_matrix"
#define PROJECTION_MATRIX "u_projection_matrix"

#define AMBIENT_COLOR "u_ambient_color"
#define LIGHT_POSITION "u_light_pos"
#define LIGHT_COLOR "u_light_color"
#define LIGHT_ITENSITY "u_light_intensity"
#define LIGHTMAP_SCALE_OFFSET "u_lightmap_scale_offset"
#define LIGHTMAP_INDEX "u_lightmap_index"

#define CAMERA_POSITION "u_camera_pos"

namespace Viry3D
{
    class Shader;
    class Renderer;
    class Light;
    class BufferObject;

    struct MaterialProperty
    {
        enum class Type
        {
            Color,
            Vector,
            Float,
            Range,
            Texture,
            Matrix,
            VectorArray,
            MatrixArray,
            Int,
            StorageBuffer,
            UniformTexelBuffer,
            StorageTexelBuffer,
        };

        union Data
        {
            float matrix[16];
            float vector[4];
            float color[4];
            float float_value;
            int int_value;
        };

        String name;
        Type type;
        Data data;
        Ref<Texture> texture;
        WeakRef<BufferObject> buffer;
        Vector<Vector4> vector_array;
        Vector<Matrix4x4> matrix_array;
        int size;
        bool dirty;
    };

    class Material : public Object
    {
    public:
        Material(const Ref<Shader>& shader);
        virtual ~Material();
        const Ref<Shader>& GetShader() const { return m_shader; }
        void SetShader(const Ref<Shader>& shader);
        int GetQueue() const;
        void SetQueue(int queue);
        void OnSetRenderer(Renderer* renderer);
        void OnUnSetRenderer(Renderer* renderer);
        const Matrix4x4* GetMatrix(const String& name) const;
        void SetMatrix(const String& name, const Matrix4x4& value);
        void SetVector(const String& name, const Vector4& value);
        void SetColor(const String& name, const Color& value);
        void SetFloat(const String& name, float value);
        void SetInt(const String& name, int value);
        void SetTexture(const String& name, const Ref<Texture>& texture);
        void SetStorageBuffer(const String& name, const Ref<BufferObject>& buffer);
        void SetUniformTexelBuffer(const String& name, const Ref<BufferObject>& buffer);
        void SetStorageTexelBuffer(const String& name, const Ref<BufferObject>& buffer);
        void SetVectorArray(const String& name, const Vector<Vector4>& array);
        void SetMatrixArray(const String& name, const Vector<Matrix4x4>& array);
        void SetLightProperties(const Ref<Light>& light);
        const Map<String, MaterialProperty>& GetProperties() const { return m_properties; }
        Ref<Texture> GetTexture(const String& name) const;
#if VR_VULKAN
        void UpdateUniformSets();
        int FindUniformSetIndex(const String& name);
        const Vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptor_sets; }
#elif VR_GLES
        void ApplyUniforms() const;
#endif

    private:
        template <class T>
        const T* GetProperty(const String& name, MaterialProperty::Type type) const
        {
            const MaterialProperty* property_ptr;
            if (m_properties.TryGet(name, &property_ptr))
            {
                if (property_ptr->type == type)
                {
                    return (const T*) &property_ptr->data;
                }
            }

            return nullptr;
        }
        template <class T>
        void SetProperty(const String& name, const T& v, MaterialProperty::Type type)
        {
            MaterialProperty* property_ptr;
            if (m_properties.TryGet(name, &property_ptr))
            {
                property_ptr->type = type;
                Memory::Copy(&property_ptr->data, &v, sizeof(v));
                property_ptr->dirty = true;
            }
            else
            {
                MaterialProperty property;
                property.name = name;
                property.type = type;
                Memory::Copy(&property.data, &v, sizeof(v));
                property.size = sizeof(v);
                property.dirty = true;
                m_properties.Add(name, property);
            }
        }
        void UpdateUniformMember(const String& name, const void* data, int size, bool& instance_cmd_dirty);
        void UpdateUniformTexture(const String& name, const Ref<Texture>& texture, bool& instance_cmd_dirty);
        void UpdateStorageBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty);
        void UpdateUniformTexelBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty);
        void UpdateStorageTexelBuffer(const String& name, const Ref<BufferObject>& buffer, bool& instance_cmd_dirty);
        void MarkRendererOrderDirty();
        void Release();

#if VR_VULKAN
        void MarkInstanceCmdDirty();
#endif

    private:
        Ref<Shader> m_shader;
        Ref<int> m_queue;
        List<Renderer*> m_renderers;
        Map<String, MaterialProperty> m_properties;
#if VR_VULKAN
        Vector<UniformSet> m_uniform_sets;
        Vector<VkDescriptorSet> m_descriptor_sets;
#endif
    };
}
