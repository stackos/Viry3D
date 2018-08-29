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

    class Renderer : public Node
    {
    public:
        Renderer();
        virtual ~Renderer();
        virtual Ref<BufferObject> GetVertexBuffer() const = 0;
        virtual Ref<BufferObject> GetIndexBuffer() const = 0;
#if VR_VULKAN
        virtual Ref<BufferObject> GetDrawBuffer() const = 0;
#endif
        virtual void Update();
        virtual void OnFrameEnd() { }
        virtual void OnResize(int width, int height) { }
        const Ref<Material>& GetMaterial() const { return m_material; }
        const Ref<Material>& GetInstanceMaterial() const { return m_instance_material; }
        void SetMaterial(const Ref<Material>& material);
        void OnAddToCamera(Camera* camera);
        void OnRemoveFromCamera(Camera* camera);
        Camera* GetCamera() const { return m_camera; }
        void MarkRendererOrderDirty();
#if VR_VULKAN
        void MarkInstanceCmdDirty();
#endif

    protected:
        virtual void OnMatrixDirty();
        void SetInstanceMatrix(const String& name, const Matrix4x4& mat);
        void SetInstanceVectorArray(const String& name, const Vector<Vector4>& array);

    private:
        Ref<Material> m_material;
        Ref<Material> m_instance_material;
        Camera* m_camera;
        bool m_model_matrix_dirty;
    };
}
