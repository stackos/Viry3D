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

#include "Main.h"
#include "Application.h"
#include "GameObject.h"
#include "graphics/Camera.h"
#include "ios/ARScene.h"

using namespace Viry3D;

class AppAR: public Application
{
public:
    AppAR()
    {
        this->SetName("Viry3D::AppAR");
        this->SetInitSize(1280, 720);
    }
    
    virtual void Start()
    {
        this->CreateFPSUI(20, 1, 1);
        
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
        camera->SetCullingMask(1 << 0);
        
#if VR_IOS
        if (ARScene::IsSupported())
        {
            m_ar = RefMake<ARScene>();
            m_ar->RunSession();
        }
#endif
    }
    
#if VR_IOS
    virtual void Update()
    {
        if (m_ar)
        {
            m_ar->UpdateSession();
            
            auto anchors = m_ar->GetAnchors();
            if (anchors.Size() > 0)
            {
                Log("anchor count:%d", anchors.Size());
            }
        }
    }
    
    virtual void OnResize(int width, int height)
    {
        Application::OnResize(width, height);
        
        if (m_ar)
        {
            m_ar->OnResize(width, height);
        }
    }
    
    virtual void OnPause()
    {
        Application::OnPause();
        
        if (m_ar)
        {
            m_ar->PauseSession();
        }
    }
    
    virtual void OnResume()
    {
        Application::OnResume();
        
        if (m_ar)
        {
            m_ar->RunSession();
        }
    }
    
    Ref<ARScene> m_ar;
#endif
};

#if 1
VR_MAIN(AppAR);
#endif
