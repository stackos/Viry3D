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

#include "MeshRenderer.h"

namespace Viry3D
{
    class SkinnedMeshRenderer : public MeshRenderer
    {
    public:
        SkinnedMeshRenderer();
        virtual ~SkinnedMeshRenderer();
        void SetBonePaths(const Vector<String>& bones) { m_bone_paths = bones; }
        void SetBonesRoot(const Ref<Node>& node) { m_bones_root = node; }

    private:
        Vector<String> m_bone_paths;
        WeakRef<Node> m_bones_root;
    };
}
