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

#include "Component.h"
#include "Material.h"
#include "container/List.h"
#include "container/Vector.h"
#include "math/Vector4.h"
#include "private/backend/DriverApi.h"

namespace Viry3D
{
    class Renderer : public Component
    {
    public:
        static const List<Renderer*>& GetRenderers() { return m_renderers; }
        Renderer();
        virtual ~Renderer();
        Ref<Material> GetMaterial() const;
        void SetMaterial(const Ref<Material>& material);
        const Vector<Ref<Material>>& GetMaterials() const { return m_materials; }
        void SetMaterials(const Vector<Ref<Material>>& materials);
        int GetLightmapIndex() const { return m_lightmap_index; }
        void SetLightmapIndex(int index);
        const Vector4& GetLightmapScaleOffset() const { return m_lightmap_scale_offset; }
        void SetLightmapScaleOffset(const Vector4& vec);
        const filament::backend::UniformBufferHandle& GetTransformUniformBuffer() const { return m_transform_uniform_buffer; }
        virtual Vector<filament::backend::RenderPrimitiveHandle> GetPrimitives();

	protected:
		virtual void PrepareRender();

	private:
		friend class Camera;

	private:
        static List<Renderer*> m_renderers;
        Vector<Ref<Material>> m_materials;
        Vector4 m_lightmap_scale_offset;
        int m_lightmap_index;
		filament::backend::UniformBufferHandle m_transform_uniform_buffer;
    };
}
