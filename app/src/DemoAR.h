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

#include "Demo.h"
#include "ios/ARScene.h"

namespace Viry3D
{
    class DemoAR : public Demo
    {
    public:
        ARScene* m_scene = nullptr;
        
        virtual void Init()
        {
            m_scene = new ARScene();
            m_scene->Run();
        }

        virtual void Done()
        {
            delete m_scene;
            m_scene = nullptr;
        }

        virtual void Update()
        {
            
        }
    };
}
