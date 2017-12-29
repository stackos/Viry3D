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

#include "LaunchScreen.h"
#include "GameObject.h"
#include "Application.h"
#include "graphics/Camera.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Texture2D.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UISprite.h"
#include "time/Timer.h"
#include "tweener/TweenUIColor.h"

namespace Viry3D
{
    DEFINE_COM_CLASS(LaunchScreen);

    void LaunchScreen::DeepCopy(const Ref<Object>& source)
    {
        Component::DeepCopy(source);
    }

    void LaunchScreen::Start()
    {
        const int layer = 31;
        const int depth = 0x7fffffff;
        const float fade_in_len = 1.0f;
        const float show_len = 1.5f;
        const float fade_out_len = 1.0f;
        auto logo = Texture2D::LoadFromFile(Application::DataPath() + "/design/logo720.png");

        auto camera = GameObject::Create("")->AddComponent<Camera>();
        camera->GetGameObject()->SetLayer(layer);
        camera->GetTransform()->SetParent(this->GetTransform());
        camera->SetCullingMask(1 << layer);
        camera->SetDepth(depth);
        camera->SetClearColor(Color(0, 0, 0, 1));
        camera->SetOrthographic(true);
        camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
        camera->SetClipNear(-1);
        camera->SetClipFar(1);

        auto canvas = GameObject::Create("")->AddComponent<UICanvasRenderer>();
        canvas->GetGameObject()->SetLayer(layer);
        canvas->GetTransform()->SetParent(this->GetTransform());
        canvas->SetAnchors(Vector2(0, 0), Vector2(0, 0));
        canvas->SetOffsets(Vector2(0, 0), Vector2((float) camera->GetTargetWidth(), (float) camera->GetTargetHeight()));
        canvas->SetPivot(Vector2(0.5f, 0.5f));
        canvas->SetSize(Vector2((float) camera->GetTargetWidth(), (float) camera->GetTargetHeight()));
        canvas->OnAnchor();
        canvas->SetSortingOrder(10000);
        canvas->GetTransform()->SetLocalPosition(Vector3::Zero());
        canvas->GetTransform()->SetLocalScale(Vector3::One());
        canvas->SetCamera(camera);

        auto sprite = GameObject::Create("")->AddComponent<UISprite>();
        sprite->GetGameObject()->SetLayer(layer);
        sprite->GetTransform()->SetParent(canvas->GetTransform());
        sprite->SetAnchors(Vector2(0.5f, 0), Vector2(0.5f, 1));
        sprite->SetOffsets(Vector2((float) -camera->GetTargetHeight() / 2, 0), Vector2((float) camera->GetTargetHeight() / 2, 0));
        sprite->SetPivot(Vector2(0.5f, 0.5f));
        sprite->OnAnchor();
        sprite->SetSingleTexture(logo);
        sprite->SetColor(Color(1, 1, 1, 0));

        auto tc = sprite->GetGameObject()->AddComponent<TweenUIColor>();
        tc->duration = fade_in_len;
        tc->from = sprite->GetColor();
        tc->to = Color(1, 1, 1, 1);
        tc->mode = TweenUIColorMode::View;

        tc = sprite->GetGameObject()->AddComponent<TweenUIColor>();
        tc->delay = fade_in_len + show_len;
        tc->duration = fade_out_len;
        tc->from = Color(1, 1, 1, 1);
        tc->to = Color(1, 1, 1, 0);
        tc->mode = TweenUIColorMode::View;
        tc->on_finish = [this]() {
            GameObject::Destroy(this->GetGameObject());
        };
    }
}
