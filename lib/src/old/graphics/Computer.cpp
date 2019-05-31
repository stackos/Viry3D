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

#include "Computer.h"
#include "BufferObject.h"

namespace Viry3D
{
    Computer::Computer()
    {

    }

    Computer::~Computer()
    {
#if VR_VULKAN
        if (m_dispatch_buffer)
        {
            m_dispatch_buffer->Destroy(Display::Instance()->GetDevice());
            m_dispatch_buffer.reset();
        }
#endif
    }

    void Computer::SetWorkgroupCount(int x, int y, int z)
    {
#if VR_VULKAN
        VkDispatchIndirectCommand dispatch;
        dispatch.x = x;
        dispatch.y = y;
        dispatch.z = z;

        if (!m_dispatch_buffer)
        {
            m_dispatch_buffer = Display::Instance()->CreateBuffer(&dispatch, sizeof(dispatch), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, VK_FORMAT_UNDEFINED);
        }
        else
        {
            Display::Instance()->UpdateBuffer(m_dispatch_buffer, 0, &dispatch, sizeof(dispatch));
        }

        this->MarkInstanceCmdDirty();
#endif
    }
}
