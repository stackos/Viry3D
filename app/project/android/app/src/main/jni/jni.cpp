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

#include <android_native_app_glue.h>
#include "Application.h"
#include "Debug.h"
#include "Input.h"
#include "io/File.h"
#include "io/Directory.h"
#include "container/List.h"
#include "container/FastList.h"
#include "thread/ThreadPool.h"
#include "graphics/Display.h"
#include "App.h"

using namespace Viry3D;

static void extract_assets_if_needed(const String& package_path, const String& data_path, bool always_extract);
static int call_activity_method_int(ANativeActivity* activity, const char* name, const char* sig, ...);
static String call_activity_method_string(ANativeActivity* activity, const char* name, const char* sig, ...);

struct TouchEvent
{
    int act;
    int index;
    int id;
    int count;
    long long time;
    float xys[20];
};

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_key_down[(int) KeyCode::COUNT];
extern bool g_key[(int) KeyCode::COUNT];
extern bool g_key_up[(int) KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static android_app* g_android_app = nullptr;
static bool g_can_draw = false;
static Display* g_display = nullptr;
static App* g_app = nullptr;
static FastList<Action> g_actions;
static Mutex g_mutex;
static int g_display_width = 0;
static int g_display_height = 0;
static bool g_paused = false;

static void get_window_size(int* width, int* height)
{
    int w = ANativeWindow_getWidth(g_android_app->window);
    int h = ANativeWindow_getHeight(g_android_app->window);
    int o = AConfiguration_getOrientation(g_android_app->config);

    if (o == ACONFIGURATION_ORIENTATION_PORT)
    {
        if (w > h)
        {
            std::swap(w, h);
        }
    }
    else if (o == ACONFIGURATION_ORIENTATION_LAND)
    {
        if (w < h)
        {
            std::swap(w, h);
        }
    }

    *width = w;
    *height = h;
}

static void engine_create()
{
	Log("engine_create begin");

	auto package_path = call_activity_method_string(g_android_app->activity, "getPackagePath", "()Ljava/lang/String;");
	auto data_files_path = call_activity_method_string(g_android_app->activity, "getFilesDirPath", "()Ljava/lang/String;");

	Log("package_path: %s", package_path.CString());
	Log("data_files_path: %s", data_files_path.CString());

	auto data_path = data_files_path + "/Assets";
	extract_assets_if_needed(package_path, data_path, true);

    String name = "viry3d-vk-demo";
    int window_width;
    int window_height;
    get_window_size(&window_width, &window_height);
    g_display_width = window_width;
    g_display_height = window_height;

    g_display = new Display(name, g_android_app->window, window_width, window_height);

    g_app = new App();
    g_app->SetName(name);
    g_app->SetDataPath(data_path);
    g_app->SetSavePath(data_path);
    g_app->Init();

	Log("engine_create end");

    g_can_draw = true;
}

static void engine_pause()
{
    Log("engine_pause");
    g_display->OnPause();
    g_can_draw = false;
}

static void engine_resume()
{
	Log("engine_resume");
    g_display->OnResume();
    g_can_draw = true;
}

static void engine_destroy()
{
	Log("engine_destroy");

    delete g_app;
    g_app = nullptr;

    delete g_display;
    g_display = nullptr;
}

static void draw()
{
    g_app->OnFrameBegin();
    g_app->Update();
    g_display->OnDraw();
    g_app->OnFrameEnd();
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
        if (!g_key[key])
        {
            g_key_down[key] = true;
            g_key[key] = true;
        }
    }
}

static void on_key_up(int key_code)
{
    int key = get_key(key_code);
    if (key >= 0)
    {
        g_key_up[key] = true;
        g_key[key] = false;
    }
}

static void touch_begin(const void* event)
{
    const TouchEvent* t = (const TouchEvent*) event;

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

static void touch_update(const void* event)
{
    const TouchEvent* t = (const TouchEvent*) event;

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
        if (touch.phase == TouchPhase::Ended)
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
    g_mutex.lock();

    for (auto i : g_actions)
    {
        if (i)
        {
            i();
        }
    }
    g_actions.Clear();

    g_mutex.unlock();
}

static void queue_event(Action action)
{
    g_mutex.lock();

    g_actions.AddLast(action);

    g_mutex.unlock();
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
                touch.xys[i * 2 + 1] = (float) g_display->GetHeight() - AMotionEvent_getY(event, i) - 1;
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
        case APP_CMD::APP_CMD_START:
            Log("APP_CMD_START");
            break;

        case APP_CMD::APP_CMD_STOP:
            Log("APP_CMD_STOP");
            break;

        case APP_CMD::APP_CMD_DESTROY:
            Log("APP_CMD_DESTROY");
            break;

        case APP_CMD::APP_CMD_INIT_WINDOW:
            Log("APP_CMD_INIT_WINDOW");
            if (g_display)
            {
                engine_resume();

                int window_width;
                int window_height;
                get_window_size(&window_width, &window_height);

                if (window_width != g_display_width || window_height != g_display_height)
                {
                    g_display_width = window_width;
                    g_display_height = window_height;

                    g_display->OnResize(window_width, window_height);
                }
            }
            else
            {
                engine_create();
            }
            break;

        case APP_CMD::APP_CMD_TERM_WINDOW:
            Log("APP_CMD_TERM_WINDOW");
            engine_pause();
            break;

        case APP_CMD::APP_CMD_CONFIG_CHANGED:
            Log("APP_CMD_CONFIG_CHANGED");
            if (g_android_app->window)
            {
                int window_width;
                int window_height;
                get_window_size(&window_width, &window_height);
                g_display_width = window_width;
                g_display_height = window_height;

                g_display->OnResize(window_width, window_height);
            }
            break;

        case APP_CMD::APP_CMD_PAUSE:
            Log("APP_CMD_PAUSE");
            g_paused = true;
            break;

        case APP_CMD::APP_CMD_RESUME:
            Log("APP_CMD_RESUME");
            g_paused = false;
            break;

		default:
			break;
	}
}

void android_main(android_app* app)
{
	Log("android_main begin");

	app->onAppCmd = handle_cmd;
	app->onInputEvent = handle_input;

    g_android_app = app;
    g_can_draw = false;

	int events;
	android_poll_source* source;

	while (true)
	{
		while (!app->destroyRequested && ALooper_pollAll(g_can_draw ? 0 : -1, nullptr, &events, (void**) &source) >= 0)
		{
			if (source != nullptr)
			{
				source->process(app, source);
			}
		}

		if (app->destroyRequested)
		{
			break;
		}

        process_events();

		if (g_can_draw && !g_paused)
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
	JNIEnv *env = nullptr;

	va_list args;
	va_start(args, sig);

	jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	jvm->AttachCurrentThread(&env, nullptr);
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
	JNIEnv *env = nullptr;

	va_list args;
	va_start(args, sig);

	jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	jvm->AttachCurrentThread(&env, nullptr);
	jclass clazz = env->GetObjectClass(object);
	jmethodID methodID = env->GetMethodID(clazz, name, sig);
	jstring str = (jstring) env->CallObjectMethodV(object, methodID, args);
	String result = env->GetStringUTFChars(str, nullptr);
	env->DeleteLocalRef(str);
	env->DeleteLocalRef(clazz);
	jvm->DetachCurrentThread();

	va_end(args);

	return result;
}

static void extract_assets(const String& source, const String& dest)
{
    auto mgr = g_android_app->activity->assetManager;
    Vector<String> files;

    // file_list.txt is generated by copy_assets.py script.
    auto asset = AAssetManager_open(mgr, "file_list.txt", AASSET_MODE_BUFFER);
    if (asset)
    {
        auto size = AAsset_getLength(asset);
        auto asset_buffer = AAsset_getBuffer(asset);
        auto buffer = ByteBuffer((byte*) asset_buffer, size);
        auto file_list = String(buffer).Replace("\r\n", "\n");
        files = file_list.Split("\n", true);

        AAsset_close(asset);
    }
    else
    {
        Log("open file_list.txt in assets failed");
    }

    for (int i = 0; i < files.Size(); i++)
    {
        const auto& file = files[i];

        asset = AAssetManager_open(mgr, file.CString(), AASSET_MODE_BUFFER);
        if (asset)
        {
            auto size = AAsset_getLength(asset);
            auto asset_buffer = AAsset_getBuffer(asset);
            auto buffer = ByteBuffer((byte*) asset_buffer, size);

            String dest_filename = dest + file.Substring(source.Size());
            auto dir = dest_filename.Substring(0, dest_filename.LastIndexOf("/"));
            Directory::Create(dir);

            File::WriteAllBytes(dest_filename, buffer);

            AAsset_close(asset);
        }
        else
        {
            Log("open asset file:%s failed", file.CString());
        }
    }
}

static void extract_assets_if_needed(const String& package_path, const String& data_path, bool always_extract)
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

		if (version != APP_VERSION_NAME)
		{
			extract = true;
		}
	}

	if (extract || always_extract)
	{
		Log("extract Assets");

        // unzip open apk file failed now,
        // i don't know why,
        // so use asset manager instead.
        //File::Unzip(package_path, "assets/Assets", data_path, true);
        extract_assets("Assets", data_path);
	}
	else
	{
		Log("skip extract Assets");
	}
}

void java_keep_screen_on(bool enable)
{
	call_activity_method_int(g_android_app->activity, "keepScreenOn", "(Z)I", enable);
}

void java_quit_application()
{
    ANativeActivity_finish(g_android_app->activity);
}

void* get_native_window()
{
	return g_android_app->window;
}
