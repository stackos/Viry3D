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

#include "Animation.h"
#include "time/Time.h"

namespace Viry3D
{
    Animation::Animation()
    {
    
    }

    Animation::~Animation()
    {
    
    }

    const String& Animation::GetClipName(int index) const
    {
        return m_clips[index].name;
    }

    void Animation::Play(int index, float fade_length)
    {
        if (m_states.Size() == 0)
        {
            fade_length = 0;
        }
        
        if (fade_length > 0.0f)
        {
            for (auto& state : m_states)
            {
                state.fade_state = FadeState::Out;
                state.fade_start_time = Time::GetTime();
                state.fade_length = fade_length;
                state.start_weight = state.weight;
            }
        }
        else
        {
            m_states.Clear();
        }

        AnimationState state;
        state.clip_index = index;
        state.play_start_time = Time::GetTime();
        state.fade_start_time = Time::GetTime();
        state.fade_length = fade_length;
        if (fade_length > 0.0f)
        {
            state.fade_state = FadeState::In;
            state.start_weight = 0.0f;
            state.weight = 0.0f;
        }
        else
        {
            state.fade_state = FadeState::Normal;
            state.start_weight = 1.0f;
            state.weight = 1.0f;
        }

        m_states.AddLast(state);
    }

    void Animation::Stop()
    {
        m_states.Clear();
    }

    void Animation::Update()
    {
        bool first_state = true;

        for (auto i = m_states.begin(); i != m_states.end(); )
        {
            auto& state = *i;
            float time = Time::GetTime() - state.play_start_time;
            const auto& clip = m_clips[state.clip_index];
            bool remove_later = false;

            if (time >= clip.length)
            {
                switch (clip.wrap_mode)
                {
                    case AnimationWrapMode::Once:
                        time = clip.length;
                        remove_later = true;
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

            float fade_time = Time::GetTime() - state.fade_start_time;
            switch (state.fade_state)
            {
                case FadeState::In:
                    if (fade_time < state.fade_length)
                    {
                        state.weight = Mathf::Lerp(state.start_weight, 1.0f, fade_time / state.fade_length);
                    }
                    else
                    {
                        state.fade_state = FadeState::Normal;
                        state.fade_start_time = Time::GetTime();
                        state.start_weight = 1.0f;
                        state.weight = 1.0f;
                    }
                    break;
                case FadeState::Normal:
                    break;
                case FadeState::Out:
                    if (fade_time < state.fade_length)
                    {
                        state.weight = Mathf::Lerp(state.start_weight, 0.0f, fade_time / state.fade_length);
                    }
                    else
                    {
                        state.weight = 0.0f;
                        remove_later = true;
                    }
                    break;
            }

            bool last_state = false;
            auto j = i;
            if (++j == m_states.end())
            {
                last_state = true;
            }

            this->Sample(state, time, state.weight, first_state, last_state);
            first_state = false;

            if (remove_later)
            {
                i = m_states.Remove(i);
            }
            else
            {
                ++i;
            }
        }
    }

    void Animation::Sample(AnimationState& state, float time, float weight, bool first_state, bool last_state)
    {
        const auto& clip = m_clips[state.clip_index];
        if (state.targets.Size() == 0)
        {
            state.targets.Resize(clip.curves.Size(), nullptr);
        }

        for (int i = 0; i < clip.curves.Size(); ++i)
        {
            const auto& curve = clip.curves[i];
            Node* target = state.targets[i];
            if (target == nullptr)
            {
                target = this->Find(curve.path).get();
                state.targets[i] = target;
                if (target)
                {
                    target->EnableNotifyChildrenOnMatrixDirty(false);
                }
                else
                {
                    continue;
                }
            }

            Vector3 local_pos;
            Quaternion local_rot;
            Vector3 local_scale;
            bool set_pos = false;
            bool set_rot = false;
            bool set_scale = false;

            for (int j = 0; j < curve.property_types.Size(); ++j)
            {
                auto type = curve.property_types[j];
                float value = curve.curves[j].Evaluate(time);

                switch (type)
                {
                    case CurvePropertyType::LocalPositionX:
                        local_pos.x = value;
                        set_pos = true;
                        break;
                    case CurvePropertyType::LocalPositionY:
                        local_pos.y = value;
                        set_pos = true;
                        break;
                    case CurvePropertyType::LocalPositionZ:
                        local_pos.z = value;
                        set_pos = true;
                        break;

                    case CurvePropertyType::LocalRotationX:
                        local_rot.x = value;
                        set_rot = true;
                        break;
                    case CurvePropertyType::LocalRotationY:
                        local_rot.y = value;
                        set_rot = true;
                        break;
                    case CurvePropertyType::LocalRotationZ:
                        local_rot.z = value;
                        set_rot = true;
                        break;
                    case CurvePropertyType::LocalRotationW:
                        local_rot.w = value;
                        set_rot = true;
                        break;

                    case CurvePropertyType::LocalScaleX:
                        local_scale.x = value;
                        set_scale = true;
                        break;
                    case CurvePropertyType::LocalScaleY:
                        local_scale.y = value;
                        set_scale = true;
                        break;
                    case CurvePropertyType::LocalScaleZ:
                        local_scale.z = value;
                        set_scale = true;
                        break;

                    case CurvePropertyType::Unknown:
                        break;
                }
            }

            if (set_pos)
            {
                Vector3 pos;
                if (first_state)
                {
                    pos = local_pos * weight;
                }
                else
                {
                    pos = target->GetLocalPosition() + local_pos * weight;
                }
                target->SetLocalPosition(pos);
            }
            if (set_rot)
            {
                Quaternion rot;
                if (first_state)
                {
                    rot = local_rot * weight;
                }
                else
                {
                    rot = Quaternion(target->GetLocalRotation());
                    if (rot.Dot(local_rot) < 0)
                    {
                        local_rot = local_rot * -1.0f;
                    }
                    rot.x += local_rot.x * weight;
                    rot.y += local_rot.y * weight;
                    rot.z += local_rot.z * weight;
                    rot.w += local_rot.w * weight;
                }
                if (last_state)
                {
                    rot.Normalize();
                }
                target->SetLocalRotation(rot);
            }
            if (set_scale)
            {
                Vector3 scale;
                if (first_state)
                {
                    scale = local_scale * weight;
                }
                else
                {
                    scale = target->GetLocalScale() + local_scale * weight;
                }
                target->SetLocalScale(scale);
            }
        }
    }
}
