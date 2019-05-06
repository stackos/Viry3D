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

#include "SkinnedMeshRenderer.h"
#include "Mesh.h"
#include "Debug.h"

namespace Viry3D
{
    SkinnedMeshRenderer::SkinnedMeshRenderer()
    {

    }

    SkinnedMeshRenderer::~SkinnedMeshRenderer()
    {

    }

    void SkinnedMeshRenderer::FindBones()
    {
        auto root = m_bones_root.lock();
        const auto& root_name = root->GetName();

        m_bones.Resize(m_bone_paths.Size());
        for (int i = 0; i < m_bones.Size(); ++i)
        {
            if (m_bone_paths[i].StartsWith(root_name))
            {
                m_bones[i] = root->Find(m_bone_paths[i].Substring(root_name.Size() + 1));
            }
            
            if (m_bones[i].expired())
            {
                Log("can not find bone: %s", m_bone_paths[i].CString());
            }
        }
    }

    void SkinnedMeshRenderer::Update()
    {
        const auto& material = this->GetMaterial();
        const auto& mesh = this->GetMesh();

        if (material && mesh && m_bone_paths.Size() > 0)
        {
            const auto& bindposes = mesh->GetBindposes();
            int bone_count = bindposes.Size();

#if VR_GLES
            int bone_max;
            if (Display::Instance()->IsGLESv3())
            {
                bone_max = 70;
            }
            else
            {
                bone_max = 30;
            }
#else
            const int bone_max = 70;
#endif  

            assert(m_bone_paths.Size() == bone_count);
            assert(m_bone_paths.Size() <= bone_max);

            if (m_bones.Empty())
            {
                this->FindBones();
            }

            Vector<Vector4> bone_vectors(bone_count * 3);

            for (int i = 0; i < bone_count; ++i)
            {
                Matrix4x4 mat = m_bones[i].lock()->GetLocalToWorldMatrix() * bindposes[i];

                bone_vectors[i * 3 + 0] = mat.GetRow(0);
                bone_vectors[i * 3 + 1] = mat.GetRow(1);
                bone_vectors[i * 3 + 2] = mat.GetRow(2);
            }

            this->SetInstanceVectorArray("u_bones", bone_vectors);
        }

        MeshRenderer::Update();
    }
}
