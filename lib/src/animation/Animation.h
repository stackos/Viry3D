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
#include "AnimationCurve.h"
#include "container/List.h"

namespace Viry3D
{
    enum class AnimationCurvePropertyType
    {
        Unknown = 0,

        LocalPositionX,
        LocalPositionY,
        LocalPositionZ,
        LocalRotationX,
        LocalRotationY,
        LocalRotationZ,
        LocalRotationW,
        LocalScaleX,
        LocalScaleY,
        LocalScaleZ,
        BlendShape,
    };
    
    struct AnimationCurveProperty
    {
        AnimationCurvePropertyType type;
        String name;
        AnimationCurve curve;
    };

    struct AnimationCurveWrapper
    {
        String path;
        Vector<AnimationCurveProperty> properties;
    };

    enum class AnimationWrapMode
    {
        Default = 0,
        Once = 1,
        Loop = 2,
        PingPong = 4,
        ClampForever = 8,
    };

    class AnimationClip : public Object
    {
	public:
		AnimationClip(): length(0), fps(0), wrap_mode(AnimationWrapMode::Default) { }
		virtual ~AnimationClip() { }

	public:
        String name;
        float length;
        float fps;
        AnimationWrapMode wrap_mode;
        Vector<AnimationCurveWrapper> curves;
    };

    enum class FadeState
    {
        In,
        Normal,
        Out,
    };

    struct AnimationState
    {
        int clip_index;
        float play_start_time;
        Vector<Transform*> targets;
        FadeState fade_state;
        float fade_start_time;
        float fade_length;
        float start_weight;
        float weight;
        float playing_time;
    };

    class Animation : public Component
    {
    public:
        Animation();
        virtual ~Animation();
		void SetClips(const Vector<Ref<AnimationClip>>& clips);
        int GetClipCount() const { return m_clips.Size(); }
        const String& GetClipName(int index) const;
        float GetClipLength(int index) const;
        int GetPlayingClip() const;
        float GetPlayingTime() const;
        void SetPlayingTime(float time);
        bool IsPaused() const { return m_paused; }
        bool IsStopped() const { return m_stopped; }
        void Play(int index, float fade_length = 0.3f);
        void Stop();
        void Pause();

    protected:
        virtual void Update();
        
    private:
        void UpdateTime();
        float GetTime();
        void Sample(AnimationState& state, float time, float weight, bool first_state, bool last_state);

    private:
        Vector<Ref<AnimationClip>> m_clips;
        List<AnimationState> m_states;
        float m_time = 0;
        float m_seek_to = -1;
        bool m_paused = false;
        bool m_stopped = true;
    };
}
