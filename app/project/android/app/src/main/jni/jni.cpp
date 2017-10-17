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

#include <android_native_app_glue.h>

#include "Application.h"
#include "Debug.h"
#include "graphics/Graphics.h"
#include "io/File.h"
#include "container/FastList.h"
#include "Input.h"

using namespace Viry3D;

struct TouchEvent
{
	int act;
    int index;
	int id;
	int count;
	long long time;
	float xys[20];
};

typedef std::function<void()> Runnable;

extern Ref<Application> viry3d_android_main();
static void extract_assets_if_needed(const String& package_path, const String& data_path);
static int call_activity_method_int(ANativeActivity* activity, const char* name, const char* sig, ...);
static String call_activity_method_string(ANativeActivity* activity, const char* name, const char* sig, ...);

extern Viry3D::Vector<Viry3D::Touch> g_input_touches;
extern Viry3D::List<Viry3D::Touch> g_input_touch_buffer;
extern bool g_key_down[(int) Viry3D::KeyCode::COUNT];
extern bool g_key[(int) Viry3D::KeyCode::COUNT];
extern bool g_key_up[(int) Viry3D::KeyCode::COUNT];
extern bool g_key_held[(int) Viry3D::KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Viry3D::Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static android_app* _app = NULL;
static bool _displayHasInit = false;
static bool _canDraw = false;
static Ref<Application> _viry3d_app;
static FastList<Runnable> _events;
static Mutex _mutex;

static void engine_create()
{
	Log("engine_create begin");

	auto package_path = call_activity_method_string(_app->activity, "getPackagePath", "()Ljava/lang/String;");
	auto data_files_path = call_activity_method_string(_app->activity, "getFilesDirPath", "()Ljava/lang/String;");

	Log("package_path: %s", package_path.CString());
	Log("data_files_path: %s", data_files_path.CString());

	auto data_path = data_files_path + "/Assets";
	extract_assets_if_needed(package_path, data_path);
	Application::SetDataPath(data_path);

	_viry3d_app = viry3d_android_main();
	_viry3d_app->OnInit();

	Log("engine_create end");

	_canDraw = true;
}

static void engine_pause()
{
    Log("engine_pause");
    _canDraw = false;
    _viry3d_app->OnPause();
}

static void engine_resume()
{
	Log("engine_resume");
	_canDraw = true;
	_viry3d_app->OnResume();
}

static void engine_destroy()
{
	Log("engine_destroy");
	_viry3d_app.reset();
}

static void draw()
{
	_viry3d_app->EnsureFPS();
	_viry3d_app->OnUpdate();
	_viry3d_app->OnDraw();
}

static int get_key(int key_code)
{
    int key = -1;

    if(key_code == 82)//KEYCODE_MENU
    {
        key = (int) KeyCode::Menu;
    }
    else if(key_code == 4)//KEYCODE_BACK
    {
        key = (int) KeyCode::Backspace;
    }
    else if(key_code == 24)//KEYCODE_VOLUME_UP
    {
        key = (int) KeyCode::PageUp;
    }
    else if(key_code == 25)//KEYCODE_VOLUME_DOWN
    {
        key = (int) KeyCode::PageDown;
    }

    return key;
}

static void on_key_down(int key_code)
{
    int key = get_key(key_code);
	if (key >= 0)
	{
		if (!g_key_held[key])
		{
			g_key_down[key] = true;
			g_key_held[key] = true;
		}
	}
}

static void on_key_up(int key_code)
{
	int key = get_key(key_code);
	if (key >= 0)
	{
		g_key_up[key] = true;
		g_key_held[key] = false;
		g_key[key] = false;
	}
}

static void touch_begin(const void *event)
{
	const TouchEvent *t = (const TouchEvent *) event;

	float x = t->xys[t->index * 2];
	float y = t->xys[t->index * 2 + 1];

	Touch touch;
	touch.deltaPosition = Vector2(0, 0);
	touch.deltaTime = 0;
	touch.time = t->time / 1000.0f;
	touch.fingerId = t->id;
	touch.phase = TouchPhase::Began;
	touch.tapCount = t->count;
	touch.position = Vector2(x, y);

	if (!g_input_touches.Empty())
	{
		g_input_touch_buffer.AddLast(touch);
	}
	else
	{
		g_input_touches.Add(touch);
	}

	if (touch.fingerId == 0)
	{
		g_mouse_button_down[0] = true;
		g_mouse_position.x = x;
		g_mouse_position.y = y;
		g_mouse_button_held[0] = true;
	}
}

static void touch_update(const void *event)
{
    const TouchEvent *t = (const TouchEvent *) event;

	float x = t->xys[t->index * 2];
	float y = t->xys[t->index * 2 + 1];

	Touch touch;
	touch.deltaPosition = Vector2(0, 0);
	touch.deltaTime = 0;
	touch.time = t->time / 1000.0f;
	touch.fingerId = t->id;
	touch.tapCount = t->count;
	touch.position = Vector2(x, y);

	if (t->act == AMOTION_EVENT_ACTION_MOVE)
	{
		touch.phase = TouchPhase::Moved;
	}
	else
	{
		touch.phase = TouchPhase::Ended;
	}

	if (!g_input_touches.Empty())
	{
		g_input_touch_buffer.AddLast(touch);
	}
	else
	{
		g_input_touches.Add(touch);
	}

	if (touch.fingerId == 0)
	{
		if (touch.phase == TouchPhase::Ended || touch.phase == TouchPhase::Canceled)
		{
			g_mouse_button_up[0] = true;
			g_mouse_position.x = x;
			g_mouse_position.y = y;
			g_mouse_button_held[0] = false;
		}
		else if (touch.phase == TouchPhase::Moved)
		{
			g_mouse_position.x = x;
			g_mouse_position.y = y;
		}
	}
}

static void on_touch(const TouchEvent& touch)
{
	switch (touch.act)
	{
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			touch_begin(&touch);
			break;

		case AMOTION_EVENT_ACTION_MOVE:
		case AMOTION_EVENT_ACTION_UP:
		case AMOTION_EVENT_ACTION_POINTER_UP:
		case AMOTION_EVENT_ACTION_CANCEL:
			touch_update(&touch);
			break;

        default:
            break;
	}
}

static void process_events()
{
    _mutex.lock();

    for (auto i = _events.Begin(); i != _events.End(); i = i->next)
    {
		if (i->value)
		{
			i->value();
		}
    }
	_events.Clear();

    _mutex.unlock();
}

static void queue_event(Runnable event)
{
	_mutex.lock();

	_events.AddLast(event);

	_mutex.unlock();
}

static int32_t handle_input(struct android_app*, AInputEvent* event)
{
	auto type = AInputEvent_getType(event);
	if (type == AINPUT_EVENT_TYPE_KEY)
	{
		auto action = AKeyEvent_getAction(event);
		auto code = AKeyEvent_getKeyCode(event);

		if (action == AKEY_EVENT_ACTION_DOWN)
		{
			queue_event([=]() {
				on_key_down(code);
			});
		}
		else if(action == AKEY_EVENT_ACTION_UP)
		{
			queue_event([=]() {
				on_key_up(code);
			});

			return 1;
		}
	}
	else if (type == AINPUT_EVENT_TYPE_MOTION)
	{
		int action = AMotionEvent_getAction(event);
		int count = (int) AMotionEvent_getPointerCount(event);
		int index = (action & 0xff00) >> 8;
        int id = AMotionEvent_getPointerId(event, (size_t) index);
		long long time = AMotionEvent_getEventTime(event);
		int act = action & 0xff;

		if (count <= 10)
		{
			TouchEvent touch;
			touch.act = act;
			touch.index = index;
            touch.id = id;
			touch.count = count;
			touch.time = time;
			for (size_t i = 0; i < count; i++)
			{
				touch.xys[i * 2] = AMotionEvent_getX(event, i);
				touch.xys[i * 2 + 1] = (float) Graphics::GetDisplay()->GetHeight() - AMotionEvent_getY(event, i) - 1;
			}

			queue_event([=](){
				on_touch(touch);
			});
		}
	}

	return 0;
}

enum class APP_CMD {
    APP_CMD_INPUT_CHANGED,
    APP_CMD_INIT_WINDOW,
    APP_CMD_TERM_WINDOW,
    APP_CMD_WINDOW_RESIZED,
    APP_CMD_WINDOW_REDRAW_NEEDED,
    APP_CMD_CONTENT_RECT_CHANGED,
    APP_CMD_GAINED_FOCUS,
    APP_CMD_LOST_FOCUS,
    APP_CMD_CONFIG_CHANGED,
    APP_CMD_LOW_MEMORY,
    APP_CMD_START,
    APP_CMD_RESUME,
    APP_CMD_SAVE_STATE,
    APP_CMD_PAUSE,
    APP_CMD_STOP,
    APP_CMD_DESTROY,
};

static void handle_cmd(android_app* app, int32_t cmdi)
{
    APP_CMD cmd = (APP_CMD) cmdi;

	switch (cmd)
	{
        case APP_CMD::APP_CMD_GAINED_FOCUS:
            if (!_displayHasInit)
            {
                _displayHasInit = true;

                engine_create();
            }
            else
            {
                engine_resume();
            }
            break;

        case APP_CMD::APP_CMD_LOST_FOCUS:
            engine_pause();
            break;

		case APP_CMD::APP_CMD_CONFIG_CHANGED:
            if (_canDraw && app->window)
            {
                int w = ANativeWindow_getWidth(app->window);
                int h = ANativeWindow_getHeight(app->window);
                _viry3d_app->OnResize(w, h);
            }
		    break;

		default:
			break;
	}
}

void android_main(android_app* app)
{
	Log("android_main begin");

	_app = app;
	app->onAppCmd = handle_cmd;
	app->onInputEvent = handle_input;

	_displayHasInit = false;
	_canDraw = false;

	int events;
	android_poll_source* source;

	while (true)
	{
		while (!app->destroyRequested && ALooper_pollAll(_canDraw ? 0 : -1, NULL, &events, (void**) &source) >= 0)
		{
			if (source != NULL)
			{
				source->process(app, source);
			}
		}

		if (app->destroyRequested)
		{
			break;
		}

        process_events();

		if (_canDraw)
		{
			draw();
		}
	}

	engine_destroy();

	Log("android_main end");
}

static int call_activity_method_int(ANativeActivity* activity, const char* name, const char* sig, ...)
{
	jobject object = activity->clazz;
	JavaVM* jvm = activity->vm;
	JNIEnv *env = NULL;

	va_list args;
	va_start(args, sig);

	jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	jvm->AttachCurrentThread(&env, NULL);
	jclass clazz = env->GetObjectClass(object);
	jmethodID methodID = env->GetMethodID(clazz, name, sig);
	int result = env->CallIntMethodV(object, methodID, args);
	env->DeleteLocalRef(clazz);
	jvm->DetachCurrentThread();

	va_end(args);

	return result;
}

static String call_activity_method_string(ANativeActivity* activity, const char* name, const char* sig, ...)
{
	jobject object = activity->clazz;
	JavaVM* jvm = activity->vm;
	JNIEnv *env = NULL;

	va_list args;
	va_start(args, sig);

	jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	jvm->AttachCurrentThread(&env, NULL);
	jclass clazz = env->GetObjectClass(object);
	jmethodID methodID = env->GetMethodID(clazz, name, sig);
	jstring str = (jstring) env->CallObjectMethodV(object, methodID, args);
	String result = env->GetStringUTFChars(str, NULL);
	env->DeleteLocalRef(str);
	env->DeleteLocalRef(clazz);
	jvm->DetachCurrentThread();

	va_end(args);

	return result;
}

static void extract_assets_if_needed(const String& package_path, const String& data_path)
{
	bool extract = false;

	auto version_file = data_path + "/version.txt";
	if (!File::Exist(version_file))
	{
		extract = true;
	}
	else
	{
		auto buffer = File::ReadAllBytes(version_file);
		auto version = String(buffer);

		if (version != APP_VERSION)
		{
			extract = true;
		}
	}

	if (extract)
	{
		Log("extract Assets");

		File::Unzip(package_path, "assets/Assets", data_path, true);
	}
	else
	{
		Log("skip extract Assets");
	}
}

void java_keep_screen_on(bool enable)
{
	call_activity_method_int(_app->activity, "keepScreenOn", "(Z)I", enable);
}

void java_quit_application()
{
	call_activity_method_int(_app->activity, "quitApplication", "()I");
}

void* get_native_window()
{
	return _app->window;
}
