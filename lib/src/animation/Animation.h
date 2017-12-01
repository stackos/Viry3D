/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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
#include "AnimationState.h"
#include "container/List.h"

namespace Viry3D
{
	enum class PlayMode
	{
		StopSameLayer = 0,
		StopAll = 4,
	};

	class Animation: public Component
	{
		DECLARE_COM_CLASS(Animation, Component);

	public:
		Animation() { }
		virtual ~Animation() { }
		void SetAnimationStates(const Map<String, AnimationState>& states) { m_states = states; }
		void FindBones();
		void Play(const String& clip, PlayMode mode = PlayMode::StopSameLayer);
		void Stop();
		void CrossFade(const String& clip, float fade_length = 0.3f, PlayMode mode = PlayMode::StopSameLayer);
		AnimationState GetAnimationState(const String& clip) const;
		void UpdateAnimationState(const String& clip, const AnimationState& state);

	private:
		struct Blend
		{
			AnimationState* state;
			float weight;

			bool operator <(const Blend& b) const
			{
				return state->layer < b.state->layer;
			}

			Blend():
				state(0),
				weight(0)
			{
			}
		};

		enum class StateCmdType
		{
			Play,
			Stop,
			CrossFade,
			UpdateState
		};

		struct StateCmd
		{
			StateCmdType type;
			String clip;
			PlayMode mode;
			float fade_length;
			AnimationState state;
		};

		virtual void Start();
		virtual void Update();
		void UpdateAnimation();
		void UpdateBlend();
		void UpdateBones();
		void UpdateBlendShapes();
		void Play(AnimationState& state);
		void Stop(AnimationState& state);
		void ExecuteStateCommands();
		void PlayCmd(const String& clip, PlayMode mode);
		void StopCmd();
		void CrossFadeCmd(const String& clip, float fade_length, PlayMode mode);

		Map<String, AnimationState> m_states;
		List<Blend> m_blends;
		Map<String, WeakRef<Transform>> m_bones;
		List<StateCmd> m_state_cmds;
	};
}
