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

#include "DeviceVulkan.h"
#include "vulkan_include.h"

namespace Viry3D
{
    class DeviceVulkanPrivate
    {
    public:
        DeviceVulkanPrivate(DeviceVulkan* device):
            m_public(device)
        {
            
        }

        DeviceVulkan* m_public;
    };

    DeviceVulkan::DeviceVulkan():
        m_private(new DeviceVulkanPrivate(this))
    {
    
    }

    DeviceVulkan::~DeviceVulkan()
    {
        delete m_private;
    }

    void DeviceVulkan::Init(int width, int height)
    {
    
    }

    void DeviceVulkan::Deinit()
    {
    
    }
}
