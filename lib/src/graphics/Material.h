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

#include "Object.h"
#include "Shader.h"
#include "Color.h"
#include "Texture.h"
#include "math/Matrix4x4.h"
#include "math/Vector4.h"
#include "math/Rect.h"
#include "container/Vector.h"
#include "container/Map.h"
#include "memory/Memory.h"
#include "private/backend/DriverApi.h"

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

#define CLIP_RECT "u_clip_rect"

namespace Viry3D
{
    class Camera;
    
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
        Vector<Vector4> vector_array;
        Vector<Matrix4x4> matrix_array;
        int size = 0;
        bool dirty;
    };
    
    struct UniformBuffer
    {
        filament::backend::UniformBufferHandle uniform_buffer;
        ByteBuffer buffer;
        bool dirty = false;
    };
    
    struct Sampler
    {
        int binding;
        Ref<Texture> texture;
    };
    
    struct SamplerGroup
    {
        filament::backend::SamplerGroupHandle sampler_group;
        Vector<Sampler> samplers;
        bool dirty = false;
    };
    
    class Material : public Object
    {
    public:
        Material(const Ref<Shader>& shader);
        virtual ~Material();
        const Ref<Shader>& GetShader() const { return m_shader; }
        int GetQueue() const;
        void SetQueue(int queue);
        const Matrix4x4* GetMatrix(const String& name) const;
        void SetMatrix(const String& name, const Matrix4x4& value);
        const Vector4* GetVector(const String& name) const;
        void SetVector(const String& name, const Vector4& value);
        void SetColor(const String& name, const Color& value);
        void SetFloat(const String& name, float value);
        void SetInt(const String& name, int value);
        Ref<Texture> GetTexture(const String& name) const;
        void SetTexture(const String& name, const Ref<Texture>& texture);
        void SetVectorArray(const String& name, const Vector<Vector4>& array);
        void SetMatrixArray(const String& name, const Vector<Matrix4x4>& array);
        const Rect& GetScissorRect() const { return m_scissor_rect; }
        void SetScissorRect(const Rect& rect);
        void Prepare();
        void Apply(const Camera* camera, int pass);
        
    private:
        template <class T>
        const T* GetProperty(const String& name, MaterialProperty::Type type) const
        {
            const MaterialProperty* ptr;
            if (m_properties.TryGet(name, &ptr))
            {
                if (ptr->type == type)
                {
                    return (const T*) &ptr->data;
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
        void UpdateUniformMember(const String& name, const void* data, int size);
        void UpdateUniformTexture(const String& name, const Ref<Texture>& texture);
        
    private:
        Ref<Shader> m_shader;
        Ref<int> m_queue;
        Map<String, MaterialProperty> m_properties;
        Rect m_scissor_rect;
        Vector<Vector<UniformBuffer>> m_unifrom_buffers;
        Vector<SamplerGroup> m_samplers;
    };
}
