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
#include "animation/AnimationCurve.h"
#include "time/Time.h"
#include "math/Mathf.h"
#include "graphics/Camera.h"

namespace Viry3D
{
	class CameraSwitcher : public Component
	{
	public:
        WeakRef<Transform> target;
        Vector<Vector3> points;
        float interval = 2.0f;
        float stability = 0.5f;
        float rotation_speed = 2.0f;
        float min_distance = 0.5f;
        AnimationCurve fov_curve = AnimationCurve::Linear(1, 30, 10, 30);
        bool auto_change = true;
        Vector3 follow_point;
        float change_time = -1;
        Vector3 current;
        
        void Init()
        {
            follow_point = target.lock()->GetPosition();
            
            if (auto_change && points.Size() > 0)
            {
                current = points[0];
            }
        }
        
        virtual void Update()
        {
            if (target.expired())
            {
                return;
            }
            
            if (auto_change && points.Size() > 0)
            {
                if (change_time < 0 || Time::GetTime() - change_time >= interval)
                {
                    change_time = Time::GetTime();
                    ChangePosition(current);
                    current = ChooseAnotherPoint(current);
                }
            }
            
            auto param = exp(-rotation_speed * Time::GetDeltaTime());
            follow_point = Vector3::Lerp(target.lock()->GetPosition(), follow_point, param);
            
            auto forward = (follow_point - this->GetTransform()->GetPosition()).Normalized();
            this->GetTransform()->SetRotation(Quaternion::LookRotation(forward, Vector3(0, 1, 0)));
        }
        
        Vector3 ChooseAnotherPoint(const Vector3& current)
        {
            while (true)
            {
                auto next = points[Mathf::RandomRange(0, points.Size())];
                auto dist = Vector3::Distance(next, target.lock()->GetPosition());
                if (next != current && dist > min_distance)
                {
                    return next;
                }
            }
        }
        
        void ChangePosition(const Vector3& destination, bool force_stable = false)
        {
            this->GetTransform()->SetPosition(destination);
            
            if (force_stable || Mathf::RandomRange(0.0f, 1.0f) < stability)
            {
                follow_point = target.lock()->GetPosition();
            }
            else
            {
                auto dir = Vector3(
                    Mathf::RandomRange(0.0f, 1.0f),
                    Mathf::RandomRange(0.0f, 1.0f),
                    Mathf::RandomRange(0.0f, 1.0f));
                if (dir.SqrMagnitude() > 0)
                {
                    dir.Normalize();
                }
                auto len = Mathf::RandomRange(0.0f, 1.0f);
                follow_point += dir * len;
            }
            
            auto dist = Vector3::Distance(target.lock()->GetPosition(), this->GetTransform()->GetPosition());
            this->GetGameObject()->GetComponent<Camera>()->SetFieldOfView(fov_curve.Evaluate(dist));
        }
	};
}
