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

#include "Label.h"
#include "CanvaRenderer.h"

namespace Viry3D
{
    Label::Label()
    {
    
    }
    
    Label::~Label()
    {
    
    }

    void Label::SetText(const String& text)
    {
        m_text = text;
        if (this->GetCanvas())
        {
            this->GetCanvas()->MarkCanvasDirty();
        }
    }

    void Label::UpdateLayout()
    {
        View::UpdateLayout();
    }

    void Label::FillSelfMeshes(Vector<ViewMesh>& meshes)
    {
        View::FillSelfMeshes(meshes);
    }
}
