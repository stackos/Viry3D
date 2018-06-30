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

        if (m_instance_material)
        {
            Map<String, Vector<MaterialProperty>> properties = m_instance_material->GetProperties();

            m_instance_material = RefMake<Material>(m_material->GetShader());
            for (const auto& i : properties)
            {
                const MaterialProperty& property = i.second[0];

                switch(property.type)
                {
                    case MaterialProperty::Type::Matrix:
                        m_instance_material->SetMatrix(property.name, *(Matrix4x4*) &property.data);
                        break;
                    case MaterialProperty::Type::Vector:
                        m_instance_material->SetVector(property.name, *(Vector4*) &property.data);
                        break;
                    case MaterialProperty::Type::Color:
                        m_instance_material->SetColor(property.name, *(Color*) &property.data);
                        break;
                    case MaterialProperty::Type::Float:
                        m_instance_material->SetFloat(property.name, *(float*) &property.data);
                        break;
                    case MaterialProperty::Type::Int:
                        m_instance_material->SetInt(property.name, *(int*) &property.data);
                        break;
                    case MaterialProperty::Type::Texture:
                        m_instance_material->SetTexture(property.name, property.texture);
                        break;
                }
            }
        }
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

    void Renderer::Update()
    {
        if (m_material)
        {
            m_material->UpdateUniformSets();
        }

        if (m_instance_material)
        {
            m_instance_material->UpdateUniformSets();
        }
    }

    void Renderer::SetInstanceMatrix(const String& name, const Matrix4x4& mat)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }
            
            m_instance_material->SetMatrix(name, mat);
        }
    }
}
