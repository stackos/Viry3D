#include "Animation.h"
#include "GameObject.h"
#include "time/Time.h"
#include "renderer/SkinnedMeshRenderer.h"
#include "Debug.h"
#include "Application.h"

namespace Viry3D
{
	DEFINE_COM_CLASS(Animation);

	void Animation::DeepCopy(const Ref<Object>& source)
	{
		Component::DeepCopy(source);

		auto src = RefCast<Animation>(source);
		m_states = src->m_states;
		m_state_cmds = src->m_state_cmds;

		this->FindBones();

		auto transform = this->GetTransform();
		auto src_transform = src->GetTransform();
		auto renderers = GetGameObject()->GetComponentsInChildren<SkinnedMeshRenderer>();
		for(auto& i : renderers)
		{
			auto& bones = i->GetBones();
			for(auto& j : bones)
			{
				auto path = j.lock()->PathInParent(src_transform);
				j = transform->Find(path);
			}
		}

		Stop();
	}

	void Animation::FindBones()
	{
		Map<String, WeakRef<Transform>> bones;
		for(const auto& i : m_states)
		{
			auto& curves = i.second.clip->curves;

			for(const auto& j : curves)
			{
				auto& path = j.first;
				bones.Add(path, WeakRef<Transform>());
			}
		}

		auto transform = GetTransform();
		for(auto& i : bones)
		{
			auto& path = i.first;
			i.second = transform->Find(path);
		}
		m_bones = bones;
	}

	void Animation::Start()
	{
	}

	void Animation::Update()
	{
		this->ExecuteStateCommands();

		Application::Current()->AddAsyncUpdateTask(
			{
				[this] ()
				{
					this->UpdateAnimation();
					return Ref<Any>();
				},
				NULL
			}
		);
	}

	void Animation::ExecuteStateCommands()
	{
		for(auto& i : m_state_cmds)
		{
			switch(i.type)
			{
				case StateCmdType::Play:
					PlayCmd(i.clip, i.mode);
					break;
				case StateCmdType::Stop:
					StopCmd();
					break;
				case StateCmdType::CrossFade:
					CrossFadeCmd(i.clip, i.fade_length, i.mode);
					break;
				case StateCmdType::UpdateState:
					m_states[i.clip] = i.state;
					break;
			}
		}
		m_state_cmds.Clear();
	}

	void Animation::UpdateAnimation()
	{
		m_blends.Clear();

		for(auto i = m_states.begin(); i != m_states.end(); i++)
		{
			AnimationState* state = &i->second;
			AnimationClip* c = state->clip.get();

			if(!state->enabled)
			{
				continue;
			}

			float now = Time::GetTime();
			float time_delta = now - state->time_last;
			state->time += time_delta * state->play_dir;
			state->time_last = now;

			if(state->fade.mode == AnimationFadeMode::In)
			{
				state->fade.weight += time_delta * (state->weight - 0) / state->fade.length;
				if(state->fade.weight >= state->weight)
				{
					state->fade.Clear();
				}
			}
			else if(state->fade.mode == AnimationFadeMode::Out)
			{
				state->fade.weight += time_delta * (0 - state->weight) / state->fade.length;
				if(state->fade.weight <= 0)
				{
					Stop(*state);
				}
			}

			if((state->play_dir == 1 && state->time > state->length) ||
				(state->play_dir == -1 && state->time < 0))
			{
				AnimationWrapMode::Enum wrap_mode = state->wrap_mode;
				if(wrap_mode == AnimationWrapMode::Default)
				{
					wrap_mode = c->wrap_mode;
				}

				switch(wrap_mode)
				{
					case AnimationWrapMode::Default:
					case AnimationWrapMode::Once:
						Stop(*state);
						continue;

					case AnimationWrapMode::Loop:
						state->time = 0;
						break;

					case AnimationWrapMode::PingPong:
						if(state->play_dir == 1)
						{
							state->play_dir = -1;
							state->time = state->length;
						}
						else
						{
							state->play_dir = 1;
							state->time = 0;
						}
						break;

					case AnimationWrapMode::ClampForever:
						state->time = state->length;
						break;
				}
			}

			if(state->enabled)
			{
				Blend blend;
				blend.state = state;
				m_blends.AddLast(blend);
			}
		}

		UpdateBlend();
		UpdateBones();
	}

	void Animation::UpdateBlend()
	{
		float full_weight = 1.0f;
		float remain_weight = 1.0f;
		int layer = 0x7fffffff;

		//compute weights
		m_blends.Sort();
		for(auto i = m_blends.begin(); i != m_blends.end(); i++)
		{
			if(remain_weight <= 0)
			{
				i->weight = 0;
				continue;
			}

			if(i->state->layer < layer)
			{
				full_weight = remain_weight;
			}
			layer = i->state->layer;

			float weight;
			if(i->state->fade.mode != AnimationFadeMode::None)
			{
				weight = full_weight * i->state->fade.weight;
			}
			else
			{
				weight = full_weight * i->state->weight;
			}

			{
				i++;
				if(i == m_blends.end())
				{
					weight = remain_weight;
				}
				i--;
			}

			if(remain_weight - weight < 0)
			{
				weight = remain_weight;
			}
			remain_weight -= weight;

			i->weight = weight;
		}
	}

	void Animation::UpdateBones()
	{
		for(auto i = m_bones.begin(); i != m_bones.end(); i++)
		{
			Vector<Vector3> poss;
			Vector<Quaternion> rots;
			Vector<Vector3> scas;
			Vector<float> weights;
			int change_mask = 0;
			float no_effect_weight = 0;

			for(auto j = m_blends.begin(); j != m_blends.end(); j++)
			{
				auto state = j->state;
				float weight = j->weight;

				CurveBinding* p_binding;
				if(state->clip->curves.TryGet(i->first, &p_binding))
				{
					Vector3 pos(0, 0, 0);
					Quaternion rot(0, 0, 0, 1);
					Vector3 sca(1, 1, 1);
					auto &cb = *p_binding;

					for(int k = 0; k < cb.curves.Size(); k++)
					{
						auto &curve = cb.curves[k];
						if(!curve.keys.Empty())
						{
							float value = curve.Evaluate(state->time);

							change_mask |= 1 << k;

							auto p = (CurveProperty::Enum) k;
							switch(p)
							{
								case CurveProperty::LocalPosX:
									pos.x = value;
									break;
								case CurveProperty::LocalPosY:
									pos.y = value;
									break;
								case CurveProperty::LocalPosZ:
									pos.z = value;
									break;

								case CurveProperty::LocalRotX:
									rot.x = value;
									break;
								case CurveProperty::LocalRotY:
									rot.y = value;
									break;
								case CurveProperty::LocalRotZ:
									rot.z = value;
									break;
								case CurveProperty::LocalRotW:
									rot.w = value;
									break;

								case CurveProperty::LocalScaX:
									sca.x = value;
									break;
								case CurveProperty::LocalScaY:
									sca.y = value;
									break;
								case CurveProperty::LocalScaZ:
									sca.z = value;
									break;

								default:
									break;
							}
						}
					}

					poss.Add(pos);
					rots.Add(rot);
					scas.Add(sca);
					weights.Add(weight);
				}
				else
				{
					no_effect_weight += weight;
				}
			}

			int in_effect_count = weights.Size();
			for(int j = 0; j < in_effect_count; j++)
			{
				float per_add = no_effect_weight / in_effect_count;
				weights[j] += per_add;
			}

			Vector3 pos_final(0, 0, 0);
			Quaternion rot_final(0, 0, 0, 0);
			Vector3 sca_final(0, 0, 0);
			for(int j = 0; j < in_effect_count; j++)
			{
				pos_final += poss[j] * weights[j];

				if(j > 0 && rots[j].Dot(rots[0]) < 0)
				{
					rots[j] = rots[j] * -1.0f;
				}
				rot_final.x += rots[j].x * weights[j];
				rot_final.y += rots[j].y * weights[j];
				rot_final.z += rots[j].z * weights[j];
				rot_final.w += rots[j].w * weights[j];

				sca_final += scas[j] * weights[j];
			}

			if(in_effect_count > 0)
			{
				auto& bone = i->second;

				if((change_mask & ((1 << 0) | (1 << 1) | (1 << 2))) != 0)
				{
					bone.lock()->SetLocalPositionDirect(pos_final);
				}

				if((change_mask & ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6))) != 0)
				{
					rot_final.Normalize();
					bone.lock()->SetLocalRotationDirect(rot_final);
				}

				if((change_mask & ((1 << 7) | (1 << 8) | (1 << 9))) != 0)
				{
					bone.lock()->SetLocalScaleDirect(sca_final);
				}
			}
		}

		GetTransform()->Changed();
	}

	AnimationState Animation::GetAnimationState(const String& clip) const
	{
		if(!m_state_cmds.Empty())
		{
			auto i = m_state_cmds.end();
			do
			{
				i--;
				if(i->type == StateCmdType::UpdateState)
				{
					return i->state;
				}
			} while(i != m_state_cmds.begin());
		}

		const AnimationState *p_state;
		if(m_states.TryGet(clip, &p_state))
		{
			return *p_state;
		}

		return AnimationState();
	}

	void Animation::UpdateAnimationState(const String& clip, const AnimationState& state)
	{
		StateCmd cmd;
		cmd.type = StateCmdType::UpdateState;
		cmd.clip = clip;
		cmd.state = state;
		m_state_cmds.AddLast(cmd);
	}

	void Animation::Play(const String &clip, PlayMode::Enum mode)
	{
		StateCmd cmd;
		cmd.type = StateCmdType::Play;
		cmd.clip = clip;
		cmd.mode = mode;
		m_state_cmds.AddLast(cmd);
	}

	void Animation::CrossFade(const String& clip, float fade_length, PlayMode::Enum mode)
	{
		StateCmd cmd;
		cmd.type = StateCmdType::CrossFade;
		cmd.clip = clip;
		cmd.fade_length = fade_length;
		cmd.mode = mode;
		m_state_cmds.AddLast(cmd);
	}

	void Animation::Stop()
	{
		StateCmd cmd;
		cmd.type = StateCmdType::Stop;
		m_state_cmds.AddLast(cmd);
	}

	void Animation::PlayCmd(const String &clip, PlayMode::Enum mode)
	{
		AnimationState* state;
		if(!m_states.TryGet(clip, &state))
		{
			return;
		}

		for(auto i = m_states.begin(); i != m_states.end(); i++)
		{
			AnimationState* s = &i->second;

			if(mode == PlayMode::StopAll && state != s && s->enabled)
			{
				Stop(*s);
			}
			else if(mode == PlayMode::StopSameLayer && s->layer == state->layer && state != s && s->enabled)
			{
				Stop(*s);
			}
			else if(state == s && !s->enabled)
			{
				Play(*s);
			}
		}
	}

	void Animation::CrossFadeCmd(const String& clip, float fade_length, PlayMode::Enum mode)
	{
		AnimationState* state;
		if(!m_states.TryGet(clip, &state))
		{
			return;
		}

		for(auto i = m_states.begin(); i != m_states.end(); i++)
		{
			AnimationState* s = &i->second;

			if(mode == PlayMode::StopAll && state != s && s->enabled)
			{
				if(s->fade.mode == AnimationFadeMode::None)
				{
					s->fade.mode = AnimationFadeMode::Out;
					s->fade.length = fade_length;
					s->fade.weight = s->weight;
				}
				else if(s->fade.mode == AnimationFadeMode::In)
				{
					s->fade.mode = AnimationFadeMode::Out;
					s->fade.length = fade_length;
				}
			}
			else if(mode == PlayMode::StopSameLayer && s->layer == state->layer && state != s && s->enabled)
			{
				if(s->fade.mode == AnimationFadeMode::None)
				{
					s->fade.mode = AnimationFadeMode::Out;
					s->fade.length = fade_length;
					s->fade.weight = s->weight;
				}
				else if(s->fade.mode == AnimationFadeMode::In)
				{
					s->fade.mode = AnimationFadeMode::Out;
					s->fade.length = fade_length;
				}
			}
			else if(state == s)
			{
				if(!s->enabled)
				{
					Play(*s);

					s->fade.mode = AnimationFadeMode::In;
					s->fade.length = fade_length;
					s->fade.weight = 0;
				}
				else
				{
					if(s->fade.mode == AnimationFadeMode::Out)
					{
						s->fade.mode = AnimationFadeMode::In;
						s->fade.length = fade_length;
					}
				}
			}
		}
	}

	void Animation::StopCmd()
	{
		for(auto &i : m_states)
		{
			Stop(i.second);
		}
	}

	void Animation::Stop(AnimationState& state)
	{
		state.enabled = false;
		state.time = 0;
		state.fade.Clear();
	}

	void Animation::Play(AnimationState& state)
	{
		state.time_last = Time::GetTime();
		state.enabled = true;
	}
}