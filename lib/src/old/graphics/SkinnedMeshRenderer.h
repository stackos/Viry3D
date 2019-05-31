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

#include "MeshRenderer.h"

namespace Viry3D
{
    class SkinnedMeshRenderer : public MeshRenderer
    {
    public:
        SkinnedMeshRenderer();
        virtual ~SkinnedMeshRenderer();
        virtual void Update();
        const Vector<String>& GetBonePaths() const { return m_bone_paths; }
        void SetBonePaths(const Vector<String>& bones) { m_bone_paths = bones; }
        Ref<Node> GetBonesRoot() const { return m_bones_root.lock(); }
        void SetBonesRoot(const Ref<Node>& node) { m_bones_root = node; }

    private:
        void FindBones();

    private:
        Vector<String> m_bone_paths;
        WeakRef<Node> m_bones_root;
        Vector<WeakRef<Node>> m_bones;
    };
}
