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

#include "AnimationClip.h"

namespace Viry3D
{
	enum class AnimationBlendMode
	{
		Blend = 0,
		Additive = 1,
	};

	enum class AnimationFadeMode
	{
		None,
		In,
		Out,
	};

	struct AnimationFade
	{
		AnimationFadeMode mode;
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
		AnimationBlendMode blend_mode;
		bool enabled;
		int layer;
		float length;
		float normalized_speed;
		float normalized_time;
		float speed;
		float time;
		float weight;
		AnimationWrapMode wrap_mode;

		float time_last;
		int play_dir;
		AnimationFade fade;
	};
}
