/*
 * Viry3D
 * Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "memory/Ref.h"
#include "container/Vector.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
    class Camera;
    class Texture2D;
    class Material;
    class GameObject;
    class Mesh;
    
    struct ARAnchor
    {
        String id;
        Matrix4x4 transform;
        Vector3 center;
        Vector3 extent;
    };
    
    class ARScene
    {
    public:
        static bool IsSupported();
        ARScene(int camera_depth = 0, int camera_culling_mask = -1, int bg_layer = 0);
        virtual ~ARScene();
        void RunSession();
        void PauseSession();
        void UpdateSession();
        void OnResize(int width, int height);
        const Ref<Camera>& GetCamera() const { return m_camera; }
        const Ref<Texture2D>& GetBackgroundTextureY() const { return m_background_texture_y; }
        const Ref<Texture2D>& GetBackgroundTextureUV() const { return m_background_texture_uv; }
        const Vector<ARAnchor>& GetAnchors() const { return m_anchors; }
        const Matrix4x4& GetCameraViewMatrix() const { return m_camera_view_matrix; }
        const Matrix4x4& GetCameraProjectionMatrix() const { return m_camera_projection_matrix; }
        const Matrix4x4& GetCameraTransform() const { return m_camera_transform; }
        
    private:
        Ref<Camera> m_camera;
        Ref<Texture2D> m_background_texture_y;
        Ref<Texture2D> m_background_texture_uv;
        Ref<Material> m_background_mat;
        Ref<GameObject> m_background_obj;
        Ref<Mesh> m_background_mesh;
        bool m_resized;
        Vector<ARAnchor> m_anchors;
        Matrix4x4 m_camera_view_matrix;
        Matrix4x4 m_camera_projection_matrix;
        Matrix4x4 m_camera_transform;
    };
}
