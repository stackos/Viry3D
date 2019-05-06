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

#include "memory/Ref.h"
#include "math/Matrix4x4.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"

namespace Viry3D
{
    class Texture;
    
    class ARScene
    {
    public:
        static bool IsSupported();
        ARScene();
        ~ARScene();
        void Run();
        void Pause();
        const Ref<Texture>& GetCameraTextureY() const;
        const Ref<Texture>& GetCameraTextureUV() const;
        const Ref<Texture>& GetEnvironmentTexture() const;
        const Matrix4x4& GetDisplayTransform() const;
        const Vector3& GetCameraPosition() const;
        const Quaternion& GetCameraRotation() const;
        float GetCameraFov() const;
        float GetCameraNear() const;
        float GetCameraFar() const;
        const Matrix4x4& GetCameraViewMatrix() const;
        const Matrix4x4& GetCameraProjectionMatrix() const;
    };
}
