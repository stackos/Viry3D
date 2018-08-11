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

#include "Animation.h"
#include "time/Time.h"

namespace Viry3D
{
    Animation::Animation():
        m_clip_index(-1),
        m_play_start_time(0)
    {
    
    }

    Animation::~Animation()
    {
    
    }

    const String& Animation::GetClipName(int index) const
    {
        return m_clips[index].name;
    }

    void Animation::Play(int index)
    {
        m_clip_index = index;
        m_play_start_time = Time::GetTime();
        m_targets.Clear();
    }

    void Animation::Stop()
    {
        m_clip_index = -1;
        m_play_start_time = 0;
        m_targets.Clear();
    }

    void Animation::Update()
    {
        if (m_clip_index >= 0)
        {
            float time = Time::GetTime() - m_play_start_time;
            const auto& clip = m_clips[m_clip_index];
            bool stop_later = false;

            if (time >= clip.length)
            {
                switch (clip.wrap_mode)
                {
                    case AnimationWrapMode::Once:
                        time = clip.length;
                        stop_later = true;
                        break;
                    case AnimationWrapMode::Loop:
                        time = fmod(time, clip.length);
                        break;
                    case AnimationWrapMode::PingPong:
                        if ((int) (time / clip.length) % 2 == 1)
                        {
                            // backward
                            time = clip.length - fmod(time, clip.length);
                        }
                        else
                        {
                            // forward
                            time = fmod(time, clip.length);
                        }
                        break;
                    case AnimationWrapMode::Default:
                    case AnimationWrapMode::ClampForever:
                        time = clip.length;
                        break;
                }
            }

            this->Sample(clip, time);

            if (stop_later)
            {
                this->Stop();
            }
        }
    }

    void Animation::Sample(const AnimationClip& clip, float time)
    {
        if (m_targets.Size() == 0)
        {
            m_targets.Resize(clip.curves.Size(), nullptr);
        }

        for (int i = 0; i < clip.curves.Size(); ++i)
        {
            const auto& curve = clip.curves[i];
            Node* target = m_targets[i];
            if (target == nullptr)
            {
                target = this->Find(curve.path).get();
                m_targets[i] = target;
                if (target)
                {
                    target->EnableNotifyChildrenOnMatrixDirty(false);
                }
                else
                {
                    continue;
                }
            }

            Vector3 local_pos = target->GetLocalPosition();
            Quaternion local_rot = target->GetLocalRotation();
            Vector3 local_scale = target->GetLocalScale();

            for (int j = 0; j < curve.property_types.Size(); ++j)
            {
                auto type = curve.property_types[j];
                float value = curve.curves[j].Evaluate(time);

                switch (type)
                {
                    case CurvePropertyType::LocalPositionX:
                        local_pos.x = value;
                        break;
                    case CurvePropertyType::LocalPositionY:
                        local_pos.y = value;
                        break;
                    case CurvePropertyType::LocalPositionZ:
                        local_pos.z = value;
                        break;

                    case CurvePropertyType::LocalRotationX:
                        local_rot.x = value;
                        break;
                    case CurvePropertyType::LocalRotationY:
                        local_rot.y = value;
                        break;
                    case CurvePropertyType::LocalRotationZ:
                        local_rot.z = value;
                        break;
                    case CurvePropertyType::LocalRotationW:
                        local_rot.w = value;
                        break;

                    case CurvePropertyType::LocalScaleX:
                        local_scale.x = value;
                        break;
                    case CurvePropertyType::LocalScaleY:
                        local_scale.y = value;
                        break;
                    case CurvePropertyType::LocalScaleZ:
                        local_scale.z = value;
                        break;
                }
            }

            target->SetLocalPosition(local_pos);
            target->SetLocalRotation(local_rot);
            target->SetLocalScale(local_scale);
        }
    }
}
