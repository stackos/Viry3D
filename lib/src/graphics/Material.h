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

namespace Viry3D
{
    class Camera;
    
	// per view uniforms, set by camera
	struct ViewUniforms
	{
		static constexpr const char* VIEW_MATRIX = "u_view_matrix";
		static constexpr const char* PROJECTION_MATRIX = "u_projection_matrix";
		static constexpr const char* CAMERA_POS = "u_camera_pos";

		Matrix4x4 view_matrix;
		Matrix4x4 projection_matrix;
		Vector4 camera_pos;
	};

	// per renderer uniforms, set by renderer
	struct RendererUniforms
	{
		static constexpr const char* MODEL_MATRIX = "u_model_matrix";
		static constexpr const char* LIGHTMAP_SCALE_OFFSET = "u_lightmap_scale_offset";
		static constexpr const char* LIGHTMAP_INDEX = "u_lightmap_index";

		Matrix4x4 model_matrix;
		Vector4 lightmap_scale_offset;
		Vector4 lightmap_index; // in x
	};

	// per renderer bones uniforms, set by skinned mesh renderer
	struct SkinnedMeshRendererUniforms
	{
		static constexpr const char* BONES = "u_bones";
		static constexpr const int BONES_VECTOR_MAX_COUNT = 210;

		Vector4 bones[BONES_VECTOR_MAX_COUNT];
	};

	// per light uniforms, set by light
	struct LightFragmentUniforms
	{
		static constexpr const char* AMBIENT_COLOR = "u_ambient_color";
		static constexpr const char* LIGHT_POS = "u_light_pos";
		static constexpr const char* LIGHT_COLOR = "u_light_color";
		static constexpr const char* LIGHT_ATTEN = "u_light_atten";
		static constexpr const char* SPOT_LIGHT_DIR = "u_spot_light_dir";
		static constexpr const char* SHADOW_PARAMS = "u_shadow_params";

		Color ambient_color;
		Vector4 light_pos;
		Color light_color; // light type in a
		Vector4 light_atten;
		Vector4 spot_light_dir;
		Vector4 shadow_params; // strength, z_bias, slope_bias, filter_radius
	};

	// per material uniforms, set by material
    struct MaterialProperty
    {
		static constexpr const char* TEXTURE = "u_texture";
		static constexpr const char* TEXTURE_SCALE_OFFSET = "u_texture_scale_offset";
		static constexpr const char* COLOR = "u_color";
		
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
		const Ref<Shader>& GetLightAddShader();
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
		void EnableKeyword(const String& keyword);
		void DisableKeyword(const String& keyword);
        void Prepare();
        void SetScissor(int target_width, int target_height);
		void Bind(int pass);
        
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
        void UpdateUniformMember(const String& name, const void* data, int size);
        void UpdateUniformTexture(const String& name, const Ref<Texture>& texture);
        
    private:
        Ref<Shader> m_shader;
		Ref<Shader> m_light_add_shader;
        Ref<int> m_queue;
        Map<String, MaterialProperty> m_properties;
        Rect m_scissor_rect;
        Vector<Vector<UniformBuffer>> m_unifrom_buffers;
        Vector<SamplerGroup> m_samplers;
    };
}
