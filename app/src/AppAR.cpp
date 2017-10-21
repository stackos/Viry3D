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
#include "graphics/Graphics.h"
#include "graphics/Texture2D.h"
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
        
        if (ARScene::IsSupported())
        {
            m_ar = RefMake<ARScene>();
            m_ar->RunSession();
            
            camera->SetPostRenderFunc([this]() {
                this->DrawBackground();
            });
        }
    }
    
    void DrawBackground()
    {
        auto texture_y = m_ar->GetBackgroundTextureY();
        if (texture_y)
        {
            Viry3D::Rect rect(0, 0, 1, 1);
            Graphics::DrawQuad(&rect, texture_y, false);
        }
    }
    
    virtual void Update()
    {
        if (m_ar)
        {
            m_ar->UpdateSession();
        }
    }
    
    virtual void OnPause()
    {
        if (m_ar)
        {
            m_ar->PauseSession();
        }
    }
    
    virtual void OnResume()
    {
        if (m_ar)
        {
            m_ar->RunSession();
        }
    }
    
    Ref<ARScene> m_ar;
};

#if 1
VR_MAIN(AppAR);
#endif
