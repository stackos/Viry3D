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

#include "Node.h"
#include "Display.h"
#include "memory/Ref.h"
#include "container/List.h"
#include "math/Matrix4x4.h"
#include "string/String.h"

namespace Viry3D
{
    class Material;
    class Camera;
    class BufferObject;

#if VR_GLES
    struct DrawBuffer
    {
        int first_index;
        int index_count;
    };
#endif

    struct RendererInstanceTransform
    {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
        Vector<Vector4> verctors;
    };

    class Renderer : public Node
    {
    public:
        Renderer();
        virtual ~Renderer();
        virtual Ref<BufferObject> GetVertexBuffer() const { return Ref<BufferObject>(); }
        virtual Ref<BufferObject> GetIndexBuffer() const { return Ref<BufferObject>(); }
#if VR_VULKAN
        Ref<BufferObject> GetDrawBuffer() const { return m_draw_buffer; }
#elif VR_GLES
        const DrawBuffer& GetDrawBuffer() const { return m_draw_buffer; }
#endif
        Ref<BufferObject> GetInstanceBuffer() const { return m_instance_buffer; }
        virtual void Update();
        virtual void OnFrameEnd() { }
        virtual void OnResize(int width, int height) { }
        const Ref<Material>& GetMaterial() const { return m_material; }
        const Ref<Material>& GetInstanceMaterial() const { return m_instance_material; }
        void SetMaterial(const Ref<Material>& material);
        int GetLightmapIndex() const { return m_lightmap_index; }
        void SetLightmapIndex(int index);
        void SetLightmapScaleOffset(const Vector4& vec);
        void OnAddToCamera(Camera* camera);
        void OnRemoveFromCamera(Camera* camera);
        Camera* GetCamera() const { return m_camera; }
        void MarkRendererOrderDirty();
#if VR_VULKAN
        void MarkInstanceCmdDirty();
#elif VR_GLES
        void OnDraw();
#endif
        void AddInstance(const Vector3& pos, const Quaternion& rot, const Vector3& scale);
        void SetInstanceTransform(int instance_index, const Vector3& pos, const Quaternion& rot, const Vector3& scale);
        void SetInstanceExtraVector(int instance_index, int vector_index, const Vector4& v);
        int GetInstanceCount() const;
        int GetInstanceStride() const;

    protected:
        virtual void OnMatrixDirty();
        virtual void UpdateDrawBuffer() { }
        void SetInstanceMatrix(const String& name, const Matrix4x4& value);
        void SetInstanceVectorArray(const String& name, const Vector<Vector4>& value);
        void SetInstanceVector(const String& name, const Vector4& value);
        void SetInstanceInt(const String& name, int value);

    private:
        void UpdateInstanceBuffer();

    protected:
#if VR_VULKAN
        Ref<BufferObject> m_draw_buffer;
#elif VR_GLES
        DrawBuffer m_draw_buffer;
#endif
        bool m_draw_buffer_dirty;

    private:
        Ref<Material> m_material;
        Ref<Material> m_instance_material;
        Camera* m_camera;
        bool m_model_matrix_dirty;
        Vector<RendererInstanceTransform> m_instances;
        Ref<BufferObject> m_instance_buffer;
        bool m_instance_buffer_dirty;
        int m_instance_extra_vector_count;
        Vector4 m_lightmap_scale_offset;
        int m_lightmap_index;
        bool m_lightmap_uv_dirty;
    };
}
