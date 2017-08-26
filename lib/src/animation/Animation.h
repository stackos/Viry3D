#pragma once

#include "Component.h"
#include "AnimationState.h"
#include "container/List.h"

namespace Viry3D
{
	struct PlayMode
	{
		enum Enum
		{
			StopSameLayer = 0,
			StopAll = 4,
		};
	};

	class Animation : public Component
	{
		DECLARE_COM_CLASS(Animation, Component);

	public:
		Animation() { }
		virtual ~Animation() { }
		void SetAnimationStates(const Map<String, AnimationState>& states) { m_states = states; }
		void FindBones();
		void Play(const String& clip, PlayMode::Enum mode = PlayMode::StopSameLayer);
		void Stop();
		void CrossFade(const String& clip, float fade_length = 0.3f, PlayMode::Enum mode = PlayMode::StopSameLayer);
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

		struct StateCmdType
		{
			enum Enum
			{
				Play,
				Stop,
				CrossFade,
				UpdateState
			};
		};

		struct StateCmd
		{
			StateCmdType::Enum type;
			String clip;
			PlayMode::Enum mode;
			float fade_length;
			AnimationState state;
		};

		virtual void Start();
		virtual void Update();
		void UpdateAnimation();
		void UpdateBlend();
		void UpdateBones();
		void Play(AnimationState& state);
		void Stop(AnimationState& state);
		void ExecuteStateCommands();
		void PlayCmd(const String& clip, PlayMode::Enum mode);
		void StopCmd();
		void CrossFadeCmd(const String& clip, float fade_length, PlayMode::Enum mode);

		Map<String, AnimationState> m_states;
		List<Blend> m_blends;
		Map<String, WeakRef<Transform>> m_bones;
		List<StateCmd> m_state_cmds;
	};
}