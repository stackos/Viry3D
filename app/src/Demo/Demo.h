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

#if VR_WINDOWS || VR_MAC || VR_WASM || VR_UWP
#define UI_SCALE 0.5
#else
#define UI_SCALE 1.0
#endif

namespace Viry3D
{
    class Demo
    {
    public:
        virtual ~Demo() { }
        virtual void Init() { }
        virtual void Done() { }
        virtual bool IsInitComplete() const { return true; }
        virtual void Update() { }
    };
}
