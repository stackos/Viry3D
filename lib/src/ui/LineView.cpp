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

#include "LineView.h"
#include "Sprite.h"

namespace Viry3D
{
    LineView::LineView():
        m_line_width(1)
    {
        
    }

    LineView::~LineView()
    {
    
    }

    void LineView::SetLineWidth(int width)
    {
        m_line_width = width;
    }

    void LineView::SetLines(const Vector<Vector2i>& lines)
    {
        m_lines = lines;
        
        this->ClearSubviews();

        int line_count = m_lines.Size() / 2;
        for (int i = 0; i < line_count; ++i)
        {
            Vector2i begin = m_lines[i * 2 + 0];
            Vector2i end = m_lines[i * 2 + 1];

            Vector2i center;
            center.x = (begin.x + end.x) / 2;
            center.y = (begin.y + end.y) / 2;
            int len = (int) sqrt((begin.x - end.x) * (begin.x - end.x) + (begin.y - end.y) * (begin.y - end.y));
            
            auto line = RefMake<Sprite>();
            line->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            line->SetPivot(Vector2(0.5f, 0.5f));
            line->SetSize(Vector2i(len, m_line_width));
            line->SetOffset(center);
            line->SetLocalRotation(Quaternion::FromToRotation(Vector3(1, 0, 0), Vector3((float) (end.x - begin.x), (float) -(end.y - begin.y), 0)));
            line->SetColor(this->GetColor());
            this->AddSubview(line);
        }
    }
}
