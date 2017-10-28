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
#include "Resource.h"
#include "graphics/Camera.h"
#include "graphics/Graphics.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UILabel.h"
#include "time/Time.h"

using namespace Viry3D;

class AppWatch: public Application
{
public:
	AppWatch()
    {
        this->SetName("Viry3D::AppWatch");
        this->SetInitSize(800, 600);
        this->SetInitFPS(5);
    }
    
	virtual void Start()
    {
        Graphics::GetDisplay()->KeepScreenOn(true);
        
        auto camera = GameObject::Create("camera")->AddComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
        camera->SetClipNear(-1);
        camera->SetClipFar(1);
        ui_camera = camera;
        
        auto scale_w = camera->GetTargetWidth() / 720.0f;
        auto scale_h = camera->GetTargetHeight() / 772.0f;
        auto scale_ui = Mathf::Min(scale_w, scale_h);
        
        auto ui = Resource::LoadGameObject("Assets/AppWatch/ui.prefab");
        ui->GetTransform()->SetScale(Vector3::One() * scale_ui);
        ui_obj = ui;
        
        fps = ui->GetTransform()->Find("fps/label")->GetGameObject()->GetComponent<Viry3D::UILabel>();
        fps->Enable(false);
        
        hour = ui->GetTransform()->Find("hour/sprite");
        minute = ui->GetTransform()->Find("minute/sprite");
        second = ui->GetTransform()->Find("second/sprite");
        day_week = ui->GetTransform()->Find("day_week/label")->GetGameObject()->GetComponent<Viry3D::UILabel>();
    }
    
	virtual void Update()
    {
        auto date = Time::GetDate();
        
        if (fps && fps->IsEnable())
        {
            fps->SetText(String::Format("fps:%d\n"
                                               "%04d.%02d.%02d "
                                               "%02d:%02d:%02d", Time::GetFPS(),
                                               date.year, date.month, date.day,
                                               date.hour, date.minute, date.second));
        }
        
        auto hour_degree = 360 * (((date.hour % 12) * 60 + date.minute) * 60 + date.second) / (float) (12 * 60 * 60);
        auto minute_degree = 360 * (date.minute * 60 + date.second) / (float) (60 * 60);
        auto second_degree = 360 * date.second / (float) 60;
        
        hour->SetLocalRotation(Quaternion::Euler(0, 0, -hour_degree));
        minute->SetLocalRotation(Quaternion::Euler(0, 0, -minute_degree));
        second->SetLocalRotation(Quaternion::Euler(0, 0, -second_degree));
        
        const char32_t ww[] = { 0x5468, 0 };
        const char32_t w0[] = { 0x65E5, 0 };
        const char32_t w1[] = { 0x4E00, 0 };
        const char32_t w2[] = { 0x4E8C, 0 };
        const char32_t w3[] = { 0x4E09, 0 };
        const char32_t w4[] = { 0x56DB, 0 };
        const char32_t w5[] = { 0x4E94, 0 };
        const char32_t w6[] = { 0x516D, 0 };
        String week_days[] = { String(w0), String(w1), String(w2), String(w3), String(w4), String(w5), String(w6) };
        
        String day_week_str = String::Format("<bold><color=#ff0000ff>%d</color></bold>  %s%s",
                                             date.day,
                                             String(ww).CString(),
                                             week_days[date.week_day].CString());
        day_week->SetText(day_week_str);
    }
    
	virtual void OnResize(int width, int height)
    {
        Application::OnResize(width, height);
        
        if (ui_camera)
        {
            auto scale_w = ui_camera->GetTargetWidth() / 720.0f;
            auto scale_h = ui_camera->GetTargetHeight() / 772.0f;
            auto scale_ui = Mathf::Min(scale_w, scale_h);
            
            ui_camera->SetOrthographicSize(ui_camera->GetTargetHeight() / 2.0f);
            ui_obj->GetTransform()->SetScale(Vector3::One() * scale_ui);
        }
    }

	Ref<Camera> ui_camera;
	Ref<GameObject> ui_obj;
	Ref<Viry3D::UILabel> fps;
	Ref<Transform> hour;
	Ref<Transform> minute;
	Ref<Transform> second;
	Ref<Viry3D::UILabel> day_week;
};

#if 0
VR_MAIN(AppWatch);
#endif
