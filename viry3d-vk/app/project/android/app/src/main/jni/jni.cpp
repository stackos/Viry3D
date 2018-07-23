/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
#include "io/File.h"
#include "io/Directory.h"
#include "container/List.h"
#include "thread/ThreadPool.h"
#include "graphics/Display.h"
#include "App.h"

using namespace Viry3D;

static void extract_assets_if_needed(const String& package_path, const String& data_path, bool always_extract);
static int call_activity_method_int(ANativeActivity* activity, const char* name, const char* sig, ...);
static String call_activity_method_string(ANativeActivity* activity, const char* name, const char* sig, ...);

static android_app* g_android_app = nullptr;
static bool g_display_init = false;
static bool g_can_draw = false;
static Display* g_display = nullptr;
static App* g_app = nullptr;

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
    int width = ANativeWindow_getWidth(g_android_app->window);
    int height = ANativeWindow_getHeight(g_android_app->window);

    Log("init display with window width: %d height: %d", width, height);

    g_display = new Display(name, g_android_app->window, width, height);

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
    g_can_draw = false;
}

static void engine_resume()
{
	Log("engine_resume");
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
            if (!g_display_init)
            {
                g_display_init = true;

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

        case APP_CMD::APP_CMD_INIT_WINDOW:
            // set keep screen on will recreate window, so recreate surface
            if (g_display_init && g_can_draw)
            {
                engine_pause();

                engine_resume();
            }
            break;

		case APP_CMD::APP_CMD_CONFIG_CHANGED:
            if (g_can_draw && app->window)
            {
                //int w = ANativeWindow_getWidth(app->window);
                //int h = ANativeWindow_getHeight(app->window);
            }
		    break;

		default:
			break;
	}
}

void android_main(android_app* app)
{
	Log("android_main begin");

	app->onAppCmd = handle_cmd;
	//app->onInputEvent = handle_input;

    g_android_app = app;
    g_display_init = false;
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

        //process_events();

		if (g_can_draw)
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

static void extract_assets(const String& source, const String& dest, bool directory)
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
        extract_assets("Assets", data_path, true);
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
	call_activity_method_int(g_android_app->activity, "quitApplication", "()I");
}

void* get_native_window()
{
	return g_android_app->window;
}
