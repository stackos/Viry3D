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

#include "Renderer.h"
#include "Camera.h"
#include "Material.h"

namespace Viry3D
{
    Renderer::Renderer()
    {
    
    }
    
    Renderer::~Renderer()
    {
    
    }

    void Renderer::SetMaterial(const Ref<Material>& material)
    {
        if (m_material)
        {
            m_material->OnUnSetRenderer(this);
        }

        m_material = material;
        this->MarkRendererOrderDirty();

        m_material->OnSetRenderer(this);

        this->MarkInstanceCmdDirty();
    }

    void Renderer::OnAddToCamera(Camera* camera)
    {
        m_cameras.AddLast(camera);
    }

    void Renderer::OnRemoveFromCamera(Camera* camera)
    {
        m_cameras.Remove(camera);
    }

    void Renderer::MarkRendererOrderDirty()
    {
        for (auto i : m_cameras)
        {
            i->MarkRendererOrderDirty();
        }
    }

    void Renderer::MarkInstanceCmdDirty()
    {
        for (auto i : m_cameras)
        {
            i->MarkInstanceCmdDirty(this);
        }
    }
}
