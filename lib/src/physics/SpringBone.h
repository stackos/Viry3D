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

#include "Component.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "container/Vector.h"

namespace Viry3D
{
    class SpringCollider;
    class SpringManager;
    
	class SpringBone : public Component
	{
    public:
        SpringBone();
        virtual ~SpringBone();
        void Init();
        void UpdateSpring();
        
    private:
        static Ref<SpringManager> GetParentSpringManager(const Ref<Transform>& t);
        
	public:
        String child_name;
        WeakRef<Transform> child;
        Vector3 bone_axis = Vector3(-1.0f, 0.0f, 0.0f);
        float radius = 0.05f;
        float stiffness_force = 0.01f;
        float drag_force = 0.4f;
        Vector3 spring_force = Vector3(0.0f, -0.0001f, 0.0f);
        Vector<String> collider_paths;
        Vector<WeakRef<SpringCollider>> colliders;
        float threshold = 0.01f;
        float spring_length = 0;
        Quaternion local_rotation;
        Vector3 curr_tip_pos;
        Vector3 prev_tip_pos;
        WeakRef<SpringManager> manager;
	};
}
