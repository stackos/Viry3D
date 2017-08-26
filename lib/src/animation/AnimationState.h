#pragma once

#include "AnimationClip.h"

namespace Viry3D
{
	struct AnimationBlendMode
	{
		enum Enum
		{
			Blend = 0,
			Additive = 1,
		};
	};

	struct AnimationFadeMode
	{
		enum Enum
		{
			None,
			In,
			Out,
		};
	};

	struct AnimationFade
	{
		AnimationFadeMode::Enum mode;
		float length;
		float weight;

		AnimationFade():
			mode(AnimationFadeMode::None),
			length(0),
			weight(0)
		{
		}

		void Clear()
		{
			mode = AnimationFadeMode::None;
			length = 0;
			weight = 0;
		}
	};

	struct AnimationState
	{
		AnimationState() { }
		AnimationState(Ref<AnimationClip>& clip):
			name(clip->GetName()),
			clip(clip),
			blend_mode(AnimationBlendMode::Blend),
			enabled(false),
			layer(0),
			length(clip->length),
			normalized_speed(1 / clip->length),
			normalized_time(0),
			speed(1),
			time(0),
			weight(1),
			wrap_mode(AnimationWrapMode::Default),
			time_last(0),
			play_dir(1)
		{
		}

		String name;
		Ref<AnimationClip> clip;
		AnimationBlendMode::Enum blend_mode;
		bool enabled;
		int layer;
		float length;
		float normalized_speed;
		float normalized_time;
		float speed;
		float time;
		float weight;
		AnimationWrapMode::Enum wrap_mode;

		float time_last;
		int play_dir;
		AnimationFade fade;
	};
}