#include <android_native_app_glue.h>

#include "Application.h"
#include "Debug.h"
#include "graphics/Graphics.h"
#include "time/Time.h"
#include "io/File.h"

using namespace Viry3D;

extern Ref<Application> viry3d_android_main();
static void extract_assets_if_needed(const String& package_path, const String& data_path);
static int call_activity_method_int(ANativeActivity* activity, const char* name, const char* sig, ...);
static String call_activity_method_string(ANativeActivity* activity, const char* name, const char* sig, ...);

android_app* _app = NULL;
bool _displayHasInit = false;
bool _canDraw = false;
static Ref<Application> _viry3d_app;

void engine_create() {
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

void engine_pause() {
    Log("engine_pause");
    _canDraw = false;
    _viry3d_app->OnPause();
}

void engine_resume() {
    Log("engine_resume");
    _canDraw = true;
    _viry3d_app->OnResume();
}

void engine_destroy() {
    Log("engine_destroy");
    _viry3d_app.reset();
}

void draw() {
	_viry3d_app->EnsureFPS();
    _viry3d_app->OnUpdate();
    _viry3d_app->OnDraw();
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    auto type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_KEY) {
        auto action = AKeyEvent_getAction(event);
        if (action == AKEY_EVENT_ACTION_UP) {
            auto code = AKeyEvent_getKeyCode(event);
            if (code == AKEYCODE_BACK) {
                //call_activity_method_int(app->activity, "backToHome", "()I");
                //return 1;
            }
        }
    }

    return 0;
}

static void handle_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (!_displayHasInit) {
                _displayHasInit = true;

                engine_create();
            } else {
                engine_resume();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_pause();
            break;
        case APP_CMD_CONFIG_CHANGED:
            {
                int w = ANativeWindow_getWidth(app->window);
                int h = ANativeWindow_getHeight(app->window);
                int screen_orientation = AConfiguration_getOrientation(app->config);
                if (screen_orientation == ACONFIGURATION_ORIENTATION_PORT) {
                    if (w > h) {
                        int temp = w;
                        w = h;
                        h = temp;
                    }
                } else if(screen_orientation == ACONFIGURATION_ORIENTATION_LAND) {
                    if (w < h) {
                        int temp = w;
                        w = h;
                        h = temp;
                    }
                }
                _viry3d_app->OnResize(w, h);
            }
            break;
        default:
            break;
    }
}

void android_main(android_app* app) {
    Log("android_main begin");

    _app = app;
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;

    _displayHasInit = false;
    _canDraw = false;

    int events;
    android_poll_source* source;

    while (true) {
        while (!app->destroyRequested && ALooper_pollAll(_canDraw ? 0 : -1, NULL, &events, (void**)&source) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }
        }

        if (app->destroyRequested) {
            break;
        }

        if (_canDraw) {
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