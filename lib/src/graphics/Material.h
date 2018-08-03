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
#include "Color.h"
#include "container/List.h"
#include "container/Map.h"
#include "math/Matrix4x4.h"
#include "math/Vector4.h"
#include "string/String.h"
#include "memory/Memory.h"

namespace Viry3D
{
    class Shader;
    class Renderer;

    struct MaterialProperty
    {
        enum class Type
        {
            Matrix,
            Vector,
            Color,
            Float,
            Int,
            Texture,
        };

        union Data
        {
            float matrix[16];
            float vector[4];
            float color[4];
            float floatValue;
            int intValue;
        };

        String name;
        Type type;
        Data data;
        Ref<Texture> texture;
        int size;
        bool dirty;
    };

    class Material
    {
    public:
        Material(const Ref<Shader>& shader);
        ~Material();
        const Ref<Shader>& GetShader() const { return m_shader; }
        void SetShader(const Ref<Shader>& shader);
        int GetQueue() const;
        void SetQueue(int queue);
        void OnSetRenderer(Renderer* renderer);
        void OnUnSetRenderer(Renderer* renderer);
        const Vector<VkDescriptorSet>& GetDescriptorSets() const { return m_descriptor_sets; }
        const Matrix4x4* GetMatrix(const String& name) const;
        void SetMatrix(const String& name, const Matrix4x4& value);
        void SetVector(const String& name, const Vector4& value);
        void SetColor(const String& name, const Color& value);
        void SetFloat(const String& name, float value);
        void SetInt(const String& name, int value);
        void SetTexture(const String& name, const Ref<Texture>& texture);
        void UpdateUniformSets();
        int FindUniformSetIndex(const String& name);
        const Map<String, MaterialProperty>& GetProperties() const { return m_properties; }

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
        void MarkRendererOrderDirty();
        void MarkInstanceCmdDirty();
        void Release();

    private:
        Ref<Shader> m_shader;
        Ref<int> m_queue;
        List<Renderer*> m_renderers;
        Vector<VkDescriptorSet> m_descriptor_sets;
        Vector<UniformSet> m_uniform_sets;
        Map<String, MaterialProperty> m_properties;
    };
}
