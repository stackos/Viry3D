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

#include "SpringBone.h"
#include "GameObject.h"
#include "SpringCollider.h"
#include "SpringManager.h"
#include "time/Time.h"

namespace Viry3D
{
    SpringBone::SpringBone()
    {
        
    }
    
    SpringBone::~SpringBone()
    {
        
    }
    
    void SpringBone::Init()
    {
        if (child.expired())
        {
            return;
        }
        
        Ref<Transform> trs = this->GetTransform();
        local_rotation = trs->GetLocalRotation();
        manager = this->GetParentSpringManager(trs);
        
        spring_length = (trs->GetPosition() - child.lock()->GetPosition()).Magnitude();
        curr_tip_pos = child.lock()->GetPosition();
        prev_tip_pos = child.lock()->GetPosition();
    }
    
    Ref<SpringManager> SpringBone::GetParentSpringManager(const Ref<Transform>& t)
    {
        Ref<SpringManager> manager = t->GetGameObject()->GetComponent<SpringManager>();
        
        if (manager)
        {
            return manager;
        }
        
        if (t->GetParent())
        {
            return GetParentSpringManager(t->GetParent());
        }
        
        return manager;
    }
    
    void SpringBone::UpdateSpring()
    {
        Ref<Transform> trs = this->GetTransform();
        trs->SetLocalRotation(local_rotation);
        
        float sqr_dt = Time::GetDeltaTime() * Time::GetDeltaTime();
        
        Vector3 force = trs->GetRotation() * (bone_axis * stiffness_force) / sqr_dt;
        force += (prev_tip_pos - curr_tip_pos) * drag_force / sqr_dt;
        force += spring_force / sqr_dt;
        
        Vector3 temp = curr_tip_pos;
        
        curr_tip_pos = (curr_tip_pos - prev_tip_pos) + curr_tip_pos + (force * sqr_dt);
        curr_tip_pos = Vector3::Normalize((curr_tip_pos - trs->GetPosition()) * spring_length) + trs->GetPosition();
        
        for (int i = 0; i < colliders.Size(); ++i)
        {
            auto collider = colliders[i].lock();
            if ((curr_tip_pos - collider->GetTransform()->GetPosition()).Magnitude() <= (radius + collider->radius))
            {
                Vector3 normal = Vector3::Normalize(curr_tip_pos - collider->GetTransform()->GetPosition());
                curr_tip_pos = collider->GetTransform()->GetPosition() + (normal * (radius + collider->radius));
                curr_tip_pos = Vector3::Normalize((curr_tip_pos - collider->GetTransform()->GetPosition()) * spring_length) + collider->GetTransform()->GetPosition();
            }
        }
        
        prev_tip_pos = temp;
        
        Vector3 aim_vector = trs->GetLocalToWorldMatrix().MultiplyDirection(bone_axis);
        Quaternion aim_rotation = Quaternion::FromToRotation(aim_vector, curr_tip_pos - trs->GetTransform()->GetPosition());
        
        Quaternion secondary_rotation = aim_rotation * trs->GetTransform()->GetRotation();
        Quaternion target_rotation = Quaternion::Lerp(trs->GetTransform()->GetRotation(), secondary_rotation, manager.lock()->dynamic_ratio);
        trs->GetTransform()->SetRotation(target_rotation);
    }
}
