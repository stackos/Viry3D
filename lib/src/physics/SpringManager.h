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
#include "SpringBone.h"
#include "GameObject.h"
#include "animation/AnimationCurve.h"

namespace Viry3D
{
	class SpringManager : public Component
	{
	public:
        float dynamic_ratio = 1.0f;
        float stiffness_force = 0.01f;
        AnimationCurve stiffness_curve;
        float drag_force = 0.4f;
        AnimationCurve drag_curve;
        Vector<String> bone_paths;
        Vector<Ref<SpringBone>> spring_bones;
        
        void Init()
        {
            spring_bones.Resize(bone_paths.Size());
            for (int i = 0; i < bone_paths.Size(); ++i)
            {
                if (bone_paths[i].Size() > 0)
                {
                    auto bone = this->GetTransform()->Find(bone_paths[i])->GetGameObject()->GetComponent<SpringBone>();
                    bone->Init();
                    spring_bones[i] = bone;
                }
            }
        }
        
        virtual void LateUpdate()
        {
            for (int i = 0; i < spring_bones.Size(); ++i)
            {
                if (dynamic_ratio > spring_bones[i]->threshold)
                {
                    spring_bones[i]->UpdateSpring();
                }
            }
        }
	};
}
