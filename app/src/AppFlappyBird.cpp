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
#include "Debug.h"
#include "Layer.h"
#include "graphics/Camera.h"
#include "graphics/Graphics.h"
#include "graphics/Display.h"
#include "ui/UICanvasRenderer.h"
#include "ui/UILabel.h"
#include "ui/UISprite.h"
#include "time/Time.h"
#include "time/Timer.h"
#include "tweener/TweenPosition.h"
#include "tweener/TweenUIColor.h"
#include "io/File.h"
#include "io/Directory.h"
#include "json/json.h"
#include "audio/AudioListener.h"
#include "audio/AudioSource.h"
#include "audio/AudioClip.h"

#define OPEN 0

using namespace Viry3D;

enum class GameState
{
	Ready,
	Play,
	Over,
	OverOut,
	ReadyIn,
};

struct Pipe
{
	int index;
	WeakRef<Transform> up;
	WeakRef<Transform> down;
	Vector3 up_start_pos;
	Vector3 down_start_pos;
	int y;
	float move;
};

static const int land_move_speed = -120;
static const int pipe_y_distance = 130;

class AppFlappyBird: public Application
{
public:
	AppFlappyBird();
	virtual void Start();
	virtual void Update();
	virtual void OnResize(int width, int height);
	void OnResize();

	void InitBG();
	void OnTouchDownBG(UIPointerEvent& e);

	void InitReady();

	void InitLand();
	void UpdateLand();

	void InitBird();
	void UpdateBirdAnim();
	void UpdateBirdMove();
	void UpdateBirdRot();

	void InitPipe();
	void GenPipe();
	void UpdatePipeGen();
	void UpdatePipeMove();

	void DetectCollision();

	void InitScore();
	void UpdateScore(int score, Vector<WeakRef<UISprite>>& sprites, int sprite_width, const String& sprite_name, int sprite_base_num, bool align_right);
	void LoadScoreBest();
	void SaveScoreBest(int score);

	void InitCover();
	void CoverFlashWhite();

	void InitOver();
	void GameOver();
	void ShowMedal(int index);

	void Restart();

	void InitAudio();

	WeakRef<Camera> m_ui_camera;
	WeakRef<GameObject> m_ui_obj;
	float m_ui_scale;

	WeakRef<UISprite> m_bird;
	int m_bird_type;
	float m_bird_frame;
	WeakRef<TweenPosition> m_bird_ready_move;
	float m_bird_speed;
	float m_bird_rot;
	int m_bird_frame_anim_speed;

	WeakRef<UISprite> m_lands[2];
	float m_land_move;
	Vector3 m_lands_start_pos[2];

	Pipe m_pipe_base;
	Vector<Pipe> m_pipes;
	float m_pipe_gen_time;
	int m_pipe_count;

	GameState m_game_state;
	float m_play_start_time;

	int m_score;
	int m_score_best;
	Vector<WeakRef<UISprite>> m_score_sprites;
	WeakRef<GameObject> m_score_obj;

	WeakRef<UISprite> m_cover_sprite;

	WeakRef<GameObject> m_over_obj;
	Vector<WeakRef<UISprite>> m_over_score_sprites;
	Vector<WeakRef<UISprite>> m_over_score_best_sprites;
	Ref<Timer> m_blink_timer;

	WeakRef<AudioSource> m_audio_source_point;
	WeakRef<AudioSource> m_audio_source_wing;
	WeakRef<AudioSource> m_audio_source_hit;
	WeakRef<AudioSource> m_audio_source_swooshing;
	WeakRef<AudioSource> m_audio_source_die;
};

#if OPEN
VR_MAIN(AppFlappyBird);
#endif

AppFlappyBird::AppFlappyBird()
{
	this->SetName("Viry3D::AppFlappyBird");
	this->SetInitSize(960 * 9 / 16, 960);
}

void AppFlappyBird::OnResize(int width, int height)
{
	Application::OnResize(width, height);

	OnResize();
}

void AppFlappyBird::OnResize()
{
	if (!m_ui_camera.expired())
	{
		auto scale_w = m_ui_camera.lock()->GetTargetWidth() / 288.0f;
		auto scale_h = m_ui_camera.lock()->GetTargetHeight() / 512.0f;
		m_ui_scale = Mathf::Max(scale_w, scale_h);

		m_ui_camera.lock()->SetOrthographicSize(m_ui_camera.lock()->GetTargetHeight() / 2.0f);
		m_ui_obj.lock()->GetTransform()->SetScale(Vector3::One() * m_ui_scale);
	}
}

void AppFlappyBird::Start()
{
	auto camera = GameObject::Create("camera")->AddComponent<Camera>();
	camera->SetCullingMask(1 << (int) Layer::UI);
	camera->SetOrthographic(true);
	camera->SetOrthographicSize(camera->GetTargetHeight() / 2.0f);
	camera->SetClipNear(-1);
	camera->SetClipFar(1);
	m_ui_camera = camera;

	auto ui = Resource::LoadGameObject("Assets/AppFlappyBird/Play.prefab");
	ui->GetTransform()->SetPosition(Vector3::Zero());
	m_ui_obj = ui;

	OnResize();

	InitBG();
	InitReady();
	InitBird();
	InitLand();
	InitPipe();
	InitScore();
	InitCover();
	InitOver();
	InitAudio();

	m_game_state = GameState::Ready;
}

void AppFlappyBird::Update()
{
	UpdateBirdAnim();
	UpdateBirdMove();
	UpdateBirdRot();
	UpdateLand();
	UpdatePipeGen();
	UpdatePipeMove();
	DetectCollision();
}

void AppFlappyBird::InitBG()
{
	int deactive = Mathf::RandomRange(0, 2);
	m_ui_obj.lock()->GetTransform()->Find(String::Format("Canvas BG/Image %d", deactive))->GetGameObject()->SetActive(false);

	int active = (deactive + 1) % 2;
	auto bg = m_ui_obj.lock()->GetTransform()->Find(String::Format("Canvas BG/Image %d", active))->GetGameObject()->GetComponent<UISprite>();
	bg->GetGameObject()->SetActive(true);
	bg->event_handler.enable = true;
	bg->event_handler.on_pointer_down = std::bind(&AppFlappyBird::OnTouchDownBG, this, std::placeholders::_1);
}

void AppFlappyBird::OnTouchDownBG(UIPointerEvent& e)
{
	if (m_game_state == GameState::Ready)
	{
		m_game_state = GameState::Play;
		m_play_start_time = Time::GetTime();
		m_pipe_gen_time = -1;
		m_bird_rot = 0;
		m_pipe_count = 0;

		auto tc = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Ready")->GetGameObject()->AddComponent<TweenUIColor>();
		tc->mode = TweenUIColorMode::View;
		tc->duration = 0.2f;
		tc->from = Color(1, 1, 1, 1);
		tc->to = Color(1, 1, 1, 0);
		tc->on_finish = [this]() {
			m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Ready")->GetGameObject()->SetActive(false);
		};

        Viry3D::Component::Destroy(m_bird_ready_move.lock());
	}

	if (m_game_state == GameState::Ready || m_game_state == GameState::Play)
	{
		const float fly_speed = 400;
		m_bird_speed = fly_speed;
		m_bird_frame_anim_speed = 14;

		m_audio_source_wing.lock()->Play();
	}
}

void AppFlappyBird::InitReady()
{
	auto ready = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Ready")->GetGameObject();
	ready->SetActive(true);
	auto vs = ready->GetComponentsInChildren<Viry3D::UIView>();
	for (auto& i : vs)
	{
		i->SetColor(Color(1, 1, 1, 1));
	}
}

void AppFlappyBird::InitBird()
{
	m_bird_frame_anim_speed = 7;

	m_bird = m_ui_obj.lock()->GetTransform()->Find("Canvas Dynamic/Image Bird")->GetGameObject()->GetComponent<UISprite>();
	m_bird_type = Mathf::RandomRange(0, 3);
	m_bird_frame = 0;
	m_bird.lock()->SetSpriteName(String::Format("bird%d_%d", m_bird_type, (int) m_bird_frame));

	m_bird.lock()->GetGameObject()->GetTransform()->SetLocalPosition(Vector3(-56, -4, 0));
	m_bird.lock()->GetGameObject()->GetTransform()->SetLocalRotation(Quaternion::Identity());

	auto move = m_bird.lock()->GetGameObject()->AddComponent<TweenPosition>();
	move->duration = 0.37f;
	move->play_style = TweenerPlayStyle::PingPong;
	move->from = m_bird.lock()->GetGameObject()->GetTransform()->GetLocalPosition();
	move->to = move->from + Vector3(0, 10, 0);
	m_bird_ready_move = move;
}

void AppFlappyBird::UpdateBirdAnim()
{
	if (m_game_state == GameState::Ready ||
		m_game_state == GameState::Play ||
		m_game_state == GameState::ReadyIn)
	{
		auto bird = m_bird.lock();
		m_bird_frame += Time::GetDeltaTime() * m_bird_frame_anim_speed;
		if ((int) m_bird_frame >= 3)
		{
			m_bird_frame = 0;
		}
		bird->SetSpriteName(String::Format("bird%d_%d", m_bird_type, (int) m_bird_frame));
	}
}

void AppFlappyBird::UpdateBirdMove()
{
	if (m_game_state == GameState::Play ||
		m_game_state == GameState::Over)
	{
		const float gravity = -1300;
		m_bird_speed += gravity * Time::GetDeltaTime();
		const float speed_min = -450;
		m_bird_speed = Mathf::Max(m_bird_speed, speed_min);

		auto birt_t = m_bird.lock()->GetTransform();
		auto pos = birt_t->GetLocalPosition();
		pos.y += m_bird_speed * Time::GetDeltaTime();

		const float bottom_y = -133;
		if (pos.y < bottom_y)
		{
			pos.y = bottom_y;

			if (m_game_state == GameState::Play)
			{
				GameOver();
			}
		}

		const float top_y = 270;
		pos.y = Mathf::Min(pos.y, top_y);

		birt_t->SetLocalPosition(pos);
	}
}

void AppFlappyBird::UpdateBirdRot()
{
	if (m_game_state == GameState::Play ||
		m_game_state == GameState::Over)
	{
		auto birt_t = m_bird.lock()->GetTransform();

		const float minus_start_speed = -350;
		if (m_bird_speed > 0)
		{
			const float rot_add_speed = 600;
			m_bird_rot += rot_add_speed * Time::GetDeltaTime();
			const float rot_max = 25;
			m_bird_rot = Mathf::Min(m_bird_rot, rot_max);
		}
		else if (m_bird_speed < minus_start_speed)
		{
			const float rot_minus_speed = -500;
			m_bird_rot += rot_minus_speed * Time::GetDeltaTime();
			const float rot_min = -90;
			m_bird_rot = Mathf::Max(m_bird_rot, rot_min);

			m_bird_frame_anim_speed = 0;
		}

		birt_t->SetLocalRotation(Quaternion::Euler(0, 0, m_bird_rot));
	}
}

void AppFlappyBird::InitLand()
{
	m_lands[0] = m_ui_obj.lock()->GetTransform()->Find("Canvas Dynamic/Image Land 0")->GetGameObject()->GetComponent<UISprite>();
	m_lands[1] = m_ui_obj.lock()->GetTransform()->Find("Canvas Dynamic/Image Land 1")->GetGameObject()->GetComponent<UISprite>();
	m_land_move = 0;
	m_lands_start_pos[0] = Vector3(24, -200, 0);
	m_lands_start_pos[1] = Vector3(360, -200, 0);
}

void AppFlappyBird::UpdateLand()
{
	if (m_game_state == GameState::Ready ||
		m_game_state == GameState::Play ||
		m_game_state == GameState::ReadyIn)
	{
		float move = Time::GetDeltaTime() * land_move_speed;
		m_land_move += move;

		const float land_move_min = -336;
		if (m_land_move <= land_move_min)
		{
			m_land_move = 0;
		}

		float moved = floor(m_land_move * m_ui_scale) / m_ui_scale;
		m_lands[0].lock()->GetTransform()->SetLocalPosition(m_lands_start_pos[0] + Vector3(moved, 0, 0));
		m_lands[1].lock()->GetTransform()->SetLocalPosition(m_lands_start_pos[1] + Vector3(moved, 0, 0));
	}
}

void AppFlappyBird::InitPipe()
{
	m_pipe_base.up = m_ui_obj.lock()->GetTransform()->Find("Canvas Dynamic/Pipes/Up");
	m_pipe_base.down = m_ui_obj.lock()->GetTransform()->Find("Canvas Dynamic/Pipes/Down");

	for (auto& i : m_pipes)
	{
		GameObject::Destroy(i.up.lock()->GetGameObject());
		GameObject::Destroy(i.down.lock()->GetGameObject());
	}
	m_pipes.Clear();
}

void AppFlappyBird::GenPipe()
{
	auto base_up = m_pipe_base.up.lock()->GetGameObject();
	auto up = GameObject::Instantiate(base_up);
	up->GetTransform()->SetParent(base_up->GetTransform()->GetParent());

	auto base_down = m_pipe_base.down.lock()->GetGameObject();
	auto down = GameObject::Instantiate(base_down);
	down->GetTransform()->SetParent(base_down->GetTransform()->GetParent());

	Pipe new_pipe;
	new_pipe.index = m_pipe_count++;
	new_pipe.up = up->GetTransform();
	new_pipe.down = down->GetTransform();

	const int pipe_y_max = 176;
	const int pipe_y_min = -64;
	new_pipe.y = Mathf::RandomRange(pipe_y_min, pipe_y_max);

	new_pipe.up_start_pos = up->GetTransform()->GetLocalPosition() + Vector3(0, (float) new_pipe.y - pipe_y_distance / 2, 0);
	new_pipe.down_start_pos = down->GetTransform()->GetLocalPosition() + Vector3(0, (float) new_pipe.y + pipe_y_distance / 2, 0);
	new_pipe.move = 0;

	m_pipes.Add(new_pipe);
}

void AppFlappyBird::UpdatePipeGen()
{
	if (m_game_state == GameState::Play)
	{
		float time = Time::GetTime();

		const float pipe_start_delay = 1;
		if (time - m_play_start_time < pipe_start_delay)
		{
			return;
		}

		const float pipe_gen_delta = 1.4f;
		if (m_pipe_gen_time < 0 || time - m_pipe_gen_time > pipe_gen_delta)
		{
			m_pipe_gen_time = time;

			GenPipe();
		}
	}
}

void AppFlappyBird::UpdatePipeMove()
{
	if (m_game_state == GameState::Play)
	{
		float move = Time::GetDeltaTime() * land_move_speed;

		for (int i = 0; i < m_pipes.Size();)
		{
			auto& pipe = m_pipes[i];

			pipe.move += move;

			const float pipe_move_min = -470;
			if (pipe.move <= pipe_move_min)
			{
				GameObject::Destroy(pipe.up.lock()->GetGameObject());
				GameObject::Destroy(pipe.down.lock()->GetGameObject());
				m_pipes.Remove(i);
				continue;
			}

			float moved = floor(pipe.move * m_ui_scale) / m_ui_scale;
			pipe.up.lock()->SetLocalPosition(pipe.up_start_pos + Vector3(moved, 0, 0));
			pipe.down.lock()->SetLocalPosition(pipe.down_start_pos + Vector3(moved, 0, 0));

			i++;
		}
	}
}

void AppFlappyBird::DetectCollision()
{
	if (m_game_state == GameState::Play)
	{
		auto bird_y = m_bird.lock()->GetTransform()->GetLocalPosition().y;

		for (int i = 0; i < m_pipes.Size(); i++)
		{
			const auto& p = m_pipes[i];

			if (p.move < -314 && p.move > -400)
			{
				if (bird_y - 12 < p.y - pipe_y_distance / 2 || bird_y + 12 > p.y + pipe_y_distance / 2)
				{
					GameOver();

					Timer::Start(0.3f)->on_tick = [this]() {
						m_audio_source_die.lock()->Play();
					};
					return;
				}
			}
			else if (p.move < -400)
			{
				int score = p.index + 1;

				if (score > m_score)
				{
					m_score = score;
					UpdateScore(m_score, m_score_sprites, 24, "font_0", 48, false);

					m_audio_source_point.lock()->Play();
				}
			}
		}
	}
}

void AppFlappyBird::InitScore()
{
	m_score = 0;
	m_score_obj = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Play/Score")->GetGameObject();
	m_score_obj.lock()->SetActive(true);

	for (int i = 1; i < m_score_sprites.Size(); i++)
	{
		GameObject::Destroy(m_score_sprites[i].lock()->GetGameObject());
	}
	m_score_sprites.Clear();

	auto score_sprite = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Play/Score/Image Score")->GetGameObject()->GetComponent<UISprite>();
	m_score_sprites.Add(score_sprite);

	UpdateScore(m_score, m_score_sprites, 24, "font_0", 48, false);

	LoadScoreBest();
}

void AppFlappyBird::LoadScoreBest()
{
	m_score_best = 0;

	auto path = Application::SavePath() + "/AppFlappyBird/save.json";
	if (File::Exist(path))
	{
		Json::Reader reader;
		Json::Value root;

		auto buffer = File::ReadAllBytes(path);
		auto str = String((const char*) buffer.Bytes(), buffer.Size());
		if (reader.parse(str.CString(), root))
		{
			m_score_best = root["score_best"].asInt();
		}
	}
}

void AppFlappyBird::SaveScoreBest(int score)
{
	m_score_best = score;
    auto dir = Application::SavePath() + "/AppFlappyBird";
    if (!Directory::Exist(dir))
    {
        Directory::Create(dir);
    }
    
	auto path = dir + "/save.json";

	Json::FastWriter writer;
	Json::Value root;

	root["score_best"] = m_score_best;

	auto json = writer.write(root);
	ByteBuffer buffer((byte*) json.c_str(), (int) json.size());
	File::WriteAllBytes(path, buffer);
}

void AppFlappyBird::UpdateScore(int score, Vector<WeakRef<UISprite>>& sprites, int sprite_width, const String& sprite_name, int sprite_base_num, bool align_right)
{
	auto score_str = String::ToString(score);
	int number_count = score_str.Size();
	for (int i = sprites.Size(); i < number_count; i++)
	{
		auto new_sprite = GameObject::Instantiate(sprites[0].lock()->GetGameObject());
		new_sprite->GetTransform()->SetParent(sprites[0].lock()->GetTransform()->GetParent());
		sprites.Add(new_sprite->GetComponent<UISprite>());
	}

	int start_x;
	if (align_right)
	{
		start_x = -(number_count - 1) * sprite_width;
	}
	else// align center
	{
		if (number_count % 2 == 1)
		{
			start_x = -number_count / 2 * sprite_width;
		}
		else
		{
			start_x = (int) ((-number_count / 2 + 0.5f) * sprite_width);
		}
	}

	for (int i = 0; i < sprites.Size(); i++)
	{
		auto sprite = sprites[i].lock();
		int x = start_x + i * sprite_width;
		sprite->GetTransform()->SetLocalPosition(Vector3((float) x, 0, 0));

		int num = score_str[i] - '0';
		sprite->SetSpriteName(String::Format("%s%d", sprite_name.CString(), sprite_base_num + num));
	}
}

void AppFlappyBird::InitCover()
{
	m_cover_sprite = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Cover")->GetGameObject()->GetComponent<UISprite>();
}

void AppFlappyBird::CoverFlashWhite()
{
	auto sprite = m_cover_sprite.lock();
	sprite->GetGameObject()->SetActive(true);
	sprite->SetColor(Color(1, 1, 1, 1));

	auto tc = sprite->GetGameObject()->AddComponent<TweenUIColor>();
	tc->mode = TweenUIColorMode::View;
	tc->duration = 0.2f;
	tc->from = Color(1, 1, 1, 1);
	tc->to = Color(1, 1, 1, 0);
	tc->on_finish = [this]() {
		m_cover_sprite.lock()->GetGameObject()->SetActive(false);
	};
}

void AppFlappyBird::InitOver()
{
	auto obj = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Over")->GetGameObject();
	obj->SetActive(false);
	obj->GetTransform()->Find("Image Panel")->SetLocalPosition(Vector3(0, -320, 0));
	obj->GetTransform()->Find("Button")->GetGameObject()->SetActive(false);
	obj->GetTransform()->Find("Image Panel/New")->GetGameObject()->SetActive(false);
	obj->GetTransform()->Find("Image Panel/Image Medal")->GetGameObject()->SetActive(false);

	for (int i = 1; i < m_over_score_sprites.Size(); i++)
	{
		GameObject::Destroy(m_over_score_sprites[i].lock()->GetGameObject());
	}
	m_over_score_sprites.Clear();
	m_over_score_sprites.Add(obj->GetTransform()->Find("Image Panel/Score/Image Score")->GetGameObject()->GetComponent<UISprite>());
	UpdateScore(0, m_over_score_sprites, 16, "number_score_0", 0, true);

	for (int i = 1; i < m_over_score_best_sprites.Size(); i++)
	{
		GameObject::Destroy(m_over_score_best_sprites[i].lock()->GetGameObject());
	}
	m_over_score_best_sprites.Clear();
	m_over_score_best_sprites.Add(obj->GetTransform()->Find("Image Panel/Best/Image Score")->GetGameObject()->GetComponent<UISprite>());
	UpdateScore(m_score_best, m_over_score_best_sprites, 16, "number_score_0", 0, true);

	m_over_obj = obj;

	WeakRef<UISprite> button_play = obj->GetTransform()->Find("Button/Image Play")->GetGameObject()->GetComponent<UISprite>();
	button_play.lock()->event_handler.enable = true;
	button_play.lock()->event_handler.on_pointer_down = [=](UIPointerEvent& e) {
		button_play.lock()->GetTransform()->SetLocalPosition(Vector3(-70, -3, 0));
	};
	button_play.lock()->event_handler.on_pointer_up = [=](UIPointerEvent& e) {
		button_play.lock()->GetTransform()->SetLocalPosition(Vector3(-70, 0, 0));
	};
	button_play.lock()->event_handler.on_pointer_click = [this](UIPointerEvent& e) {
		Restart();

		m_audio_source_swooshing.lock()->Play();
	};
}

void AppFlappyBird::Restart()
{
	if (m_game_state == GameState::Over)
	{
		m_game_state = GameState::OverOut;

		auto sprite = m_cover_sprite.lock();
		sprite->GetGameObject()->SetActive(true);
		sprite->SetColor(Color(0, 0, 0, 0));

		auto tc = sprite->GetGameObject()->AddComponent<TweenUIColor>();
		tc->mode = TweenUIColorMode::View;
		tc->duration = 0.3f;
		tc->from = Color(0, 0, 0, 0);
		tc->to = Color(0, 0, 0, 1);
		tc->on_finish = [this]() {
			auto tc = m_cover_sprite.lock()->GetGameObject()->AddComponent<TweenUIColor>();
			tc->mode = TweenUIColorMode::View;
			tc->duration = 0.3f;
			tc->from = Color(0, 0, 0, 1);
			tc->to = Color(0, 0, 0, 0);
			tc->on_finish = [this]() {
				m_cover_sprite.lock()->GetGameObject()->SetActive(false);
				m_game_state = GameState::Ready;
			};

			InitBG();
			InitReady();
			InitBird();
			InitLand();
			InitPipe();
			InitScore();
			InitCover();
			InitOver();
			m_game_state = GameState::ReadyIn;

			if (m_blink_timer)
			{
				Timer::Stop(m_blink_timer);
				m_blink_timer.reset();
			}
		};
	}
}

void AppFlappyBird::GameOver()
{
	m_game_state = GameState::Over;

	CoverFlashWhite();

	m_audio_source_hit.lock()->Play();

	Timer::Start(0.5f)->on_tick = [=]() {
		m_score_obj.lock()->SetActive(false);

		// show game over
		{
			auto over_obj = m_over_obj.lock();
			over_obj->SetActive(true);

			auto tp = over_obj->GetTransform()->Find("Image Title")->GetGameObject()->AddComponent<TweenPosition>();
			tp->duration = 0.2f;
			tp->from = tp->GetTransform()->GetLocalPosition();
			tp->to = tp->from + Vector3(0, 10, 0);
			tp->curve.keys.Clear();
			tp->curve.keys.Add(Keyframe(0, 0, 1, 1));
			tp->curve.keys.Add(Keyframe(0.5f, 1, 1, -1));
			tp->curve.keys.Add(Keyframe(1, 0, -1, -1));

			auto tc = over_obj->GetTransform()->Find("Image Title")->GetGameObject()->AddComponent<TweenUIColor>();
			tc->mode = TweenUIColorMode::View;
			tc->GetGameObject()->GetComponent<Viry3D::UIView>()->SetColor(Color(1, 1, 1, 0));
			tc->duration = 0.2f;
			tc->from = Color(1, 1, 1, 0);
			tc->to = Color(1, 1, 1, 1);

			m_audio_source_swooshing.lock()->Play();

			tp = over_obj->GetTransform()->Find("Image Panel")->GetGameObject()->AddComponent<TweenPosition>();
			tp->duration = 0.3f;
			tp->delay = 0.7f;
			tp->from = tp->GetTransform()->GetLocalPosition();
			tp->to = Vector3(0, 0, 0);
			tp->curve.keys.Clear();
			tp->curve.keys.Add(Keyframe(0, 0, 2, 2));
			tp->curve.keys.Add(Keyframe(1, 1, 0, 0));

			Timer::Start(0.7f)->on_tick = [this]() {
				m_audio_source_swooshing.lock()->Play();
			};

			Timer::Start(1.2f)->on_tick = [=]() {
				if (m_score > 0)
				{
					float tick = 1.0f / m_score;

					auto t = Timer::Start(tick, true);
					t->on_tick = [=]() {
						int score = t->tick_count;
						UpdateScore(score, m_over_score_sprites, 16, "number_score_0", 0, true);

						if (score == m_score)
						{
							Timer::Stop(t);

							if (m_score > m_score_best)
							{
								m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Over/Image Panel/New")->GetGameObject()->SetActive(true);
								UpdateScore(m_score, m_over_score_best_sprites, 16, "number_score_0", 0, true);
								SaveScoreBest(m_score);
							}

							if (m_score >= 10 && m_score < 20)
							{
								ShowMedal(3);
							}
							else if (m_score >= 20 && m_score < 40)
							{
								ShowMedal(2);
							}
							else if (m_score >= 40 && m_score < 80)
							{
								ShowMedal(1);
							}
							else if (m_score >= 80)
							{
								ShowMedal(0);
							}

							Timer::Start(0.3f)->on_tick = [=]() {
								over_obj->GetTransform()->Find("Button")->GetGameObject()->SetActive(true);
							};
						}
					};
				}
				else
				{
					Timer::Start(0.3f)->on_tick = [=]() {
						over_obj->GetTransform()->Find("Button")->GetGameObject()->SetActive(true);
					};
				}
			};
		}
	};
}

void AppFlappyBird::ShowMedal(int index)
{
	WeakRef<GameObject> obj = m_ui_obj.lock()->GetTransform()->Find("Canvas UI/Over/Image Panel/Image Medal")->GetGameObject();
	obj.lock()->SetActive(true);
	obj.lock()->GetComponent<UISprite>()->SetSpriteName(String::Format("medals_%d", index));
	obj.lock()->GetTransform()->Find("Blink")->GetGameObject()->SetActive(false);

	m_blink_timer = Timer::Start(0.5f, true);
	m_blink_timer->on_tick = [=]() {
		WeakRef<GameObject> blink_obj = obj.lock()->GetTransform()->Find("Blink")->GetGameObject();
		blink_obj.lock()->SetActive(true);
		WeakRef<UISprite> blink = blink_obj.lock()->GetComponent<UISprite>();
		blink.lock()->SetSpriteName("blink_00");

		blink_obj.lock()->GetTransform()->SetLocalPosition(Vector3(Mathf::RandomRange(-22.0f, 22.0f), Mathf::RandomRange(-22.0f, 22.0f), 0));

		auto t = Timer::Start(0.1f, true);
		t->on_tick = [=]() {
			auto tick_count = t->tick_count;
			if (tick_count == 3)
			{
				Timer::Stop(t);
				blink_obj.lock()->SetActive(false);
			}
			else
			{
				blink.lock()->SetSpriteName(String::Format("blink_0%d", tick_count));
			}
		};
	};
}

void AppFlappyBird::InitAudio()
{
	GameObject::Create("AudioListener")->AddComponent<AudioListener>();

	m_audio_source_point = GameObject::Create("")->AddComponent<AudioSource>();
	auto clip = AudioClip::LoadFromFile(Application::DataPath() + "/AppFlappyBird/sfx_point.wav");
	m_audio_source_point.lock()->SetClip(clip);

	m_audio_source_wing = GameObject::Create("")->AddComponent<AudioSource>();
	clip = AudioClip::LoadFromFile(Application::DataPath() + "/AppFlappyBird/sfx_wing.wav");
	m_audio_source_wing.lock()->SetClip(clip);

	m_audio_source_hit = GameObject::Create("")->AddComponent<AudioSource>();
	clip = AudioClip::LoadFromFile(Application::DataPath() + "/AppFlappyBird/sfx_hit.wav");
	m_audio_source_hit.lock()->SetClip(clip);

	m_audio_source_swooshing = GameObject::Create("")->AddComponent<AudioSource>();
	clip = AudioClip::LoadFromFile(Application::DataPath() + "/AppFlappyBird/sfx_swooshing.wav");
	m_audio_source_swooshing.lock()->SetClip(clip);

	m_audio_source_die = GameObject::Create("")->AddComponent<AudioSource>();
	clip = AudioClip::LoadFromFile(Application::DataPath() + "/AppFlappyBird/sfx_die.wav");
	m_audio_source_die.lock()->SetClip(clip);
}
