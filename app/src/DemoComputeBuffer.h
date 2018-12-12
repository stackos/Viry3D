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

#include "DemoMesh.h"
#include "graphics/Computer.h"

namespace Viry3D
{
    class DemoComputeBuffer : public DemoMesh
    {
    public:
        void InitCompute()
        {
            String cs = R"(#version 310 es
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (constant_id = 0) const int BUFFER_ELEMENTS = 32;
layout (binding = 0) buffer Pos
{
   int values[];
};

int fibonacci(int n)
{
	if (n <= 1)
    {
		return n;
	}
	int curr = 1;
	int prev = 1;
	for (int i = 2; i < n; ++i)
    {
		int temp = curr;
		curr += prev;
		prev = temp;
	}
	return curr;
}

void main() 
{
	int index = int(gl_GlobalInvocationID.x);
	if (index >= BUFFER_ELEMENTS) return;	
	values[index] = fibonacci(values[index]);
}
)";

            auto shader = RefMake<Shader>(cs);
            auto material = RefMake<Material>(shader);

            auto computer = RefMake<Computer>();
            computer->SetMaterial(material);
            computer->SetWorkgroupCount(32, 1, 1);

            m_camera->AddRenderer(computer);
        }

        virtual void Init()
        {
            this->InitCamera();
            this->InitUI();
            this->InitCompute();
        }

        virtual void Done()
        {
            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
