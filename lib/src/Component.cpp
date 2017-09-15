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

#include "Component.h"
#include "GameObject.h"

#include "graphics/Camera.h"
#include "graphics/Light.h"
#include "graphics/RenderTextureBliter.h"
#include "renderer/MeshRenderer.h"
#include "renderer/SkinnedMeshRenderer.h"
#include "renderer/ParticleSystemRenderer.h"
#include "renderer/ParticleSystem.h"
#include "renderer/Terrain.h"
#include "animation/Animation.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UISprite.h"
#include "ui/UILabel.h"
#include "postprocess/ImageEffect.h"
#include "postprocess/ImageEffectBlur.h"
#include "tweener/TweenPosition.h"
#include "tweener/TweenUIColor.h"
#include "time/Timer.h"
#include "audio/AudioListener.h"
#include "audio/AudioSource.h"

namespace Viry3D
{
	DEFINE_COM_BASE(Component);

	void Component::RegisterComponents()
	{
		Transform::RegisterComponent();
		Camera::RegisterComponent();
		MeshRenderer::RegisterComponent();
		SkinnedMeshRenderer::RegisterComponent();
		ParticleSystemRenderer::RegisterComponent();
		ParticleSystem::RegisterComponent();
		Terrain::RegisterComponent();
		Animation::RegisterComponent();
		UICanvasRenderer::RegisterComponent();
		UIView::RegisterComponent();
		UISprite::RegisterComponent();
		UILabel::RegisterComponent();
		ImageEffect::RegisterComponent();
		ImageEffectBlur::RegisterComponent();
		TweenPosition::RegisterComponent();
		TweenUIColor::RegisterComponent();
		Timer::RegisterComponent();
		AudioListener::RegisterComponent();
		AudioSource::RegisterComponent();
		Light::RegisterComponent();
		RenderTextureBliter::RegisterComponent();
	}

	Component::Component():
		m_deleted(false),
		m_started(false),
		m_enable(true)
	{
	}

	Ref<GameObject> Component::GetGameObject() const
	{
		return m_gameobject.lock();
	}

	Ref<Transform> Component::GetTransform() const
	{
		return m_transform.lock();
	}

	void Component::Destroy(Ref<Component> com)
	{
		if (com)
		{
			com->Delete();
		}
	}

	void Component::DeepCopy(const Ref<Object>& source)
	{
		Object::DeepCopy(source);

		auto src = RefCast<Component>(source);
		m_deleted = src->m_deleted;
		m_enable = src->m_enable;
		m_started = false;
	}

	void Component::SetName(const String& name)
	{
		if (GetName() != name)
		{
			Object::SetName(name);
			GetGameObject()->SetName(name);
		}
	}

	void Component::Delete()
	{
		if (!m_deleted)
		{
			m_deleted = true;
			Enable(false);
		}
	}

	void Component::Enable(bool enable)
	{
		if (m_enable != enable)
		{
			m_enable = enable;

			bool obj_active = GetGameObject()->IsActiveInHierarchy();
			if (obj_active)
			{
				if (m_enable)
				{
					OnEnable();
				}
				else
				{
					OnDisable();
				}
			}
		}
	}

	Ref<Component> Component::GetRef() const
	{
		return GetGameObject()->GetComponentRef(this);
	}

	bool Component::IsComponent(String type) const
	{
		auto& names = GetClassNames();

		for (auto& i : names)
		{
			if (i == type)
			{
				return true;
			}
		}

		return false;
	}
}
