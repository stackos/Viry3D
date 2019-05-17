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

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include "Engine.h"
#include "Debug.h"
#include "Input.h"
#include "io/File.h"
#include "io/Directory.h"
#include "container/List.h"
#include "container/FastList.h"

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

static JNIEnv* g_env;
static jobject g_jni_obj;
static Engine* g_engine = nullptr;
static bool g_paused = false;

static int call_activity_method_int(const char* name, const char* sig, ...);
static String call_activity_method_string(const char* name, const char* sig, ...);
static void extract_assets_if_needed(jobject asset_manager, const String& package_path, const String& data_path, bool always_extract);
static void on_key_down(int key_code);
static void on_key_up(int key_code);
static void on_touch(const TouchEvent& touch);
static void swap_bytes(void* bytes, int size);

extern "C"
{
    void Java_com_viry3d_lib_JNI_engineCreate(JNIEnv* env, jobject obj, jobject surface, jint width, jint height, jobject asset_manager)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineCreate begin");
        Log("surface w: %d h: %d", width, height);

        auto package_path = call_activity_method_string("getPackagePath", "()Ljava/lang/String;");
        auto data_files_path = call_activity_method_string("getFilesDirPath", "()Ljava/lang/String;");

        Log("package_path: %s", package_path.CString());
        Log("data_files_path: %s", data_files_path.CString());

	    auto data_path = data_files_path + "/Assets";
        extract_assets_if_needed(asset_manager, package_path, data_path, true);

        ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
        g_engine = Engine::Create(window, width, height);
        g_engine->SetDataPath(data_path);
        g_engine->SetSavePath(data_path);

        g_paused = false;

        Log("engineCreate end");
    }

    void Java_com_viry3d_lib_JNI_engineDestroy(JNIEnv* env, jobject obj)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineDestroy begin");

        Engine::Destroy(&g_engine);

        Log("engineDestroy end");
    }

    void Java_com_viry3d_lib_JNI_engineSurfaceResize(JNIEnv* env, jobject obj, jobject surface, jint width, jint height)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineSurfaceResize begin");
        Log("surface w: %d h: %d", width, height);

        ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
        g_engine->OnResize(window, width, height);

        g_paused = false;

        Log("engineSurfaceResize end");
    }

    void Java_com_viry3d_lib_JNI_engineSurfaceDestroy(JNIEnv* env, jobject obj)
    {
        g_env = env;
        g_jni_obj = obj;

        g_paused = true;

        Log("engineSurfaceDestroy");
    }

    void Java_com_viry3d_lib_JNI_enginePause(JNIEnv* env, jobject obj)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("enginePause");
    }

    void Java_com_viry3d_lib_JNI_engineResume(JNIEnv* env, jobject obj)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineResume");
    }

    void Java_com_viry3d_lib_JNI_engineDraw(JNIEnv* env, jobject obj)
    {
        g_env = env;
        g_jni_obj = obj;

        if (!g_paused)
        {
            g_engine->Execute();
        }
    }

    void Java_com_viry3d_lib_JNI_engineKeyDown(JNIEnv* env, jobject obj, jint key_code)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineKeyDown: %d", key_code);

        on_key_down(key_code);
    }

    void Java_com_viry3d_lib_JNI_engineKeyUp(JNIEnv* env, jobject obj, jint key_code)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineKeyUp: %d", key_code);

        on_key_up(key_code);
    }

    void Java_com_viry3d_lib_JNI_engineTouch(JNIEnv* env, jobject obj, jbyteArray touch_data)
    {
        g_env = env;
        g_jni_obj = obj;

        Log("engineTouch");

        jsize data_size = env->GetArrayLength(touch_data);
        jbyte* data = env->GetByteArrayElements(touch_data, nullptr);
        if (data != nullptr)
        {
            TouchEvent e;
            memcpy(&e, data, sizeof(e) - sizeof(e.xys));
            swap_bytes(&e.act, sizeof(e.act));
            swap_bytes(&e.index, sizeof(e.index));
            swap_bytes(&e.id, sizeof(e.id));
            swap_bytes(&e.count, sizeof(e.count));
            swap_bytes(&e.time, sizeof(e.time));

            if (e.count <= 10)
            {
                int offset = sizeof(e) - sizeof(e.xys);
                memcpy(e.xys, &data[offset], sizeof(float) * 2 * e.count);

                for (int i = 0; i < e.count; ++i)
                {
                    float x = e.xys[i * 2];
                    float y = e.xys[i * 2 + 1];
                    swap_bytes(&x, sizeof(x));
                    swap_bytes(&y, sizeof(y));
                    y = (float) g_engine->GetHeight() - y - 1;
                    e.xys[i * 2] = x;
                    e.xys[i * 2 + 1] = y;
                }

                on_touch(e);
            }

            env->ReleaseByteArrayElements(touch_data, data, JNI_ABORT);
        }
    }
}

#define AMOTION_EVENT_ACTION_DOWN 0
#define AMOTION_EVENT_ACTION_UP 1
#define AMOTION_EVENT_ACTION_MOVE 2
#define AMOTION_EVENT_ACTION_CANCEL 3
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP 6

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_key_down[(int) KeyCode::COUNT];
extern bool g_key[(int) KeyCode::COUNT];
extern bool g_key_up[(int) KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static FastList<Action> g_actions;
static Mutex g_mutex;

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
        if (g_input_touch_buffer.Empty())
        {
            if (g_input_touches[0].phase == TouchPhase::Moved && g_input_touches[0].fingerId == touch.fingerId)
            {
                g_input_touches[0] = touch;
            }
            else
            {
                g_input_touch_buffer.AddLast(touch);
            }
        }
        else
        {
            if (g_input_touch_buffer.Last().phase == TouchPhase::Moved && g_input_touch_buffer.Last().fingerId == touch.fingerId)
            {
                g_input_touch_buffer.Last() = touch;
            }
            else
            {
                g_input_touch_buffer.AddLast(touch);
            }
        }
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

static void swap_bytes(void* bytes, int size)
{
    char* p = (char*) bytes;
    int count = size / 2;
    for (int i = 0; i < count; ++i)
    {
        char t = p[i];
        p[i] = p[size - 1 - i];
        p[size - 1 - i] = t;
    }
}

static int call_activity_method_int(const char* name, const char* sig, ...)
{
    va_list args;
	va_start(args, sig);

	jclass clazz = g_env->GetObjectClass(g_jni_obj);
    jmethodID methodID = g_env->GetMethodID(clazz, name, sig);
    int result = g_env->CallIntMethodV(g_jni_obj, methodID, args);
    g_env->DeleteLocalRef(clazz);

	va_end(args);

	return result;
}

static String call_activity_method_string(const char* name, const char* sig, ...)
{
    va_list args;
	va_start(args, sig);

	jclass clazz = g_env->GetObjectClass(g_jni_obj);
    jmethodID methodID = g_env->GetMethodID(clazz, name, sig);
    jstring str = (jstring) g_env->CallObjectMethodV(g_jni_obj, methodID, args);
    String result = g_env->GetStringUTFChars(str, nullptr);
    g_env->DeleteLocalRef(str);
    g_env->DeleteLocalRef(clazz);

	va_end(args);

	return result;
}

static void extract_assets(jobject asset_manager, const String& source, const String& dest)
{
    auto mgr = AAssetManager_fromJava(g_env, asset_manager);
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

static void extract_assets_if_needed(jobject asset_manager, const String& package_path, const String& data_path, bool always_extract)
{
	bool extract = false;

	auto version_file = data_path + "/version.txt";
	if (!File::Exist(version_file))
	{
		extract = true;
	}
	else
	{
		auto version = File::ReadAllText(version_file);

		if (version != VR_VERSION_NAME)
		{
			extract = true;
		}
	}

	if (extract || always_extract)
	{
		Log("extract Assets");

        // unzip open apk file failed now,
        // so use asset manager instead.
        //File::Unzip(package_path, "assets/Assets", data_path, true);
        extract_assets(asset_manager, "Assets", data_path);

        File::WriteAllText(version_file, VR_VERSION_NAME);
	}
	else
	{
		Log("skip extract Assets");
	}
}

void java_quit_application()
{
    call_activity_method_int("quitApplication", "()I");
}
