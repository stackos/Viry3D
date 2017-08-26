#pragma once

#include "Component.h"
#include "GameObject.h"
#include "animation/AnimationCurve.h"
#include <functional>

namespace Viry3D
{
	struct TweenerPlayStyle
	{
		enum Enum
		{
			Once,
			Loop,
			PingPong,
		};
	};

	class Tweener : public Component
	{
		DECLARE_COM_CLASS_ABSTRACT(Tweener, Component);
	protected:
		Tweener();
		virtual void Update();
		virtual void OnSetValue(float value) = 0;

	public:
		AnimationCurve curve;
		float duration;
		float delay;
		TweenerPlayStyle::Enum play_style;
		std::function<void()> on_finish;

	protected:
		float m_time_start;
		float m_time;
		bool m_reverse;
		bool m_finish;
	};
}