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

#include "Object.h"

namespace Viry3D
{
    class GameObject;
    class Transform;
    
    class Component : public Object
    {
    public:
        Component();
        virtual ~Component();
        Ref<GameObject> GetGameObject() const { return m_object.lock(); }
        const Ref<Transform>& GetTransform() const;
        void Enable(bool enable);
        bool IsEnable() const { return m_enable; }

    protected:
        virtual void Start() { }
        virtual void Update() { }
        virtual void LateUpdate() { }
        virtual void OnTransformDirty() { }
        virtual void OnEnable(bool enable) { }
        virtual void OnGameObjectLayerChanged() { }
        virtual void OnGameObjectActiveChanged() { }
        
	private:
        friend class GameObject;
        
    private:
        WeakRef<GameObject> m_object;
        bool m_enable = true;
        bool m_started = false;
    };
}
