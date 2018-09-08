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

#include <wrl.h>
#include <ppltasks.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <angle_windowsstore.h>

#include "graphics/Display.h"
#include "App.h"
#include "Debug.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Microsoft::WRL;
using namespace Platform;

static Viry3D::String ConvertString(Platform::String^ src)
{
    unsigned int size = src->Length();
    const wchar_t* data16 = src->Data();
    char32_t* data32 = new char32_t[size + 1];
    for (unsigned int i = 0; i < size; ++i)
    {
        data32[i] = data16[i];
    }
    data32[size] = 0;
    return Viry3D::String(data32);
}

static Platform::String^ ConvertString(const Viry3D::String& src)
{
    auto data32 = src.ToUnicode32();
    int size = data32.Size();
    wchar_t* data16 = new wchar_t[size + 1];
    for (int i = 0; i < size; ++i)
    {
        data16[i] = (wchar_t) data32[i];
    }
    data16[size] = 0;
    return ref new Platform::String(data16);
}

namespace Viry3D
{
    bool FileExist(const String& path)
    {
        Ref<bool> exist;

        String root = Application::Instance()->GetDataPath().Replace("/", "\\");
        String local_path = path.Substring(root.Size() + 1).Replace("/", "\\");

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(root))).then([&](StorageFolder^ assets) {
            return assets->TryGetItemAsync(ConvertString(local_path));
        }).then([&](IStorageItem^ item) {
            exist = RefMake<bool>(item != nullptr);
        });

        while (!exist)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }

        return *exist;
    }

    ByteBuffer FileReadAllBytes(const String& path)
    {
        Ref<ByteBuffer> bufer;

        String root = Application::Instance()->GetDataPath().Replace("/", "\\");
        String local_path = path.Substring(root.Size() + 1).Replace("/", "\\");

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(root))).then([&](StorageFolder^ assets) {
            return assets->TryGetItemAsync(ConvertString(local_path));
        }).then([&](IStorageItem^ item) {
            return FileIO::ReadBufferAsync((StorageFile^) item);
        }).then([&](Streams::IBuffer^ ib) {
            auto reader = Streams::DataReader::FromBuffer(ib);
            bufer = RefMake<ByteBuffer>(ib->Length);
            reader->ReadBytes(Platform::ArrayReference<byte>(bufer->Bytes(), bufer->Size()));
        });

        while (!bufer)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }

        return *bufer;
    }
}


namespace app
{
    ref class App sealed: public Windows::ApplicationModel::Core::IFrameworkView
    {
    public:
        App()
        {
        }

        // IFrameworkView Methods.
        virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
        {
            // Register event handlers for app lifecycle. This example includes Activated, so that we
            // can make the CoreWindow active and start rendering on the window.
            applicationView->Activated +=
                ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

            // Logic for other event handlers could go here.
            // Information about the Suspending and Resuming event handlers can be found here:
            // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx
        }

        virtual void SetWindow(Windows::UI::Core::CoreWindow^ window)
        {
            window->VisibilityChanged +=
                ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

            window->Closed +=
                ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

            // The CoreWindow has been created, so EGL can be initialized.
            this->InitEGL(window);
        }

        virtual void Load(Platform::String^ entryPoint)
        {
            this->RecreateRenderer();
        }

        virtual void Run()
        {
            while (!m_window_closed)
            {
                if (m_window_visible)
                {
                    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

                    EGLint window_width = 0;
                    EGLint window_height = 0;
                    eglQuerySurface(m_egl_display, m_egl_surface, EGL_WIDTH, &window_width);
                    eglQuerySurface(m_egl_display, m_egl_surface, EGL_HEIGHT, &window_height);

                    this->Draw(window_width, window_height);

                    // The call to eglSwapBuffers might not be successful (e.g. due to Device Lost)
                    // If the call fails, then we must reinitialize EGL and the GL resources.
                    if (eglSwapBuffers(m_egl_display, m_egl_surface) != GL_TRUE)
                    {
                        this->DoneEGL();
                        this->InitEGL(CoreWindow::GetForCurrentThread());
                        this->RecreateRenderer();
                    }
                }
                else
                {
                    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
                }
            }

            this->DoneRenderer();
            this->DoneEGL();
        }

        virtual void Uninitialize()
        {
        }

    private:
        void InitEGL(Windows::UI::Core::CoreWindow^ window)
        {
            const EGLint config_attribs[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_NONE
            };

            EGLint context_attribs[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };

            const EGLint surface_attribs[] =
            {
                // EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER is part of the same optimization as EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER (see above).
                // If you have compilation issues with it then please update your Visual Studio templates.
                EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
                EGL_NONE
            };

            const EGLint default_display_attribs[] =
            {
                // These are the default display attributes, used to request ANGLE's D3D11 renderer.
                // eglInitialize will only succeed with these attributes if the hardware supports D3D11 Feature Level 10_0+.
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

                // EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER is an optimization that can have large performance benefits on mobile devices.
                // Its syntax is subject to change, though. Please update your Visual Studio templates if you experience compilation issues with it.
                EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,

                // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that enables ANGLE to automatically call 
                // the IDXGIDevice3::Trim method on behalf of the application when it gets suspended. 
                // Calling IDXGIDevice3::Trim when an application is suspended is a Windows Store application certification requirement.
                EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
                EGL_NONE,
            };

            const EGLint level_9_3_display_attribs[] =
            {
                // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature Level 9_3.
                // These attributes are used if the call to eglInitialize fails with the default display attributes.
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
                EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
                EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
                EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
                EGL_NONE,
            };

            const EGLint warp_display_attribs[] =
            {
                // These attributes can be used to request D3D11 WARP.
                // They are used if eglInitialize fails with both the default display attributes and the 9_3 display attributes.
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
                EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
                EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
                EGL_NONE,
            };

            EGLConfig config = nullptr;

            // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in display attributes, used to configure D3D11.
            PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
            if (!eglGetPlatformDisplayEXT)
            {
                throw Exception::CreateException(E_FAIL, L"Failed to get function eglGetPlatformDisplayEXT");
            }

            //
            // To initialize the display, we make three sets of calls to eglGetPlatformDisplayEXT and eglInitialize, with varying 
            // parameters passed to eglGetPlatformDisplayEXT:
            // 1) The first calls uses "defaultDisplayAttributes" as a parameter. This corresponds to D3D11 Feature Level 10_0+.
            // 2) If eglInitialize fails for step 1 (e.g. because 10_0+ isn't supported by the default GPU), then we try again 
            //    using "fl9_3DisplayAttributes". This corresponds to D3D11 Feature Level 9_3.
            // 3) If eglInitialize fails for step 2 (e.g. because 9_3+ isn't supported by the default GPU), then we try again 
            //    using "warpDisplayAttributes".  This corresponds to D3D11 Feature Level 11_0 on WARP, a D3D11 software rasterizer.
            //

            // This tries to initialize EGL to D3D11 Feature Level 10_0+. See above comment for details.
            m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, default_display_attribs);
            if (m_egl_display == EGL_NO_DISPLAY)
            {
                throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
            }

            if (eglInitialize(m_egl_display, NULL, NULL) == EGL_FALSE)
            {
                // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is unavailable (e.g. on some mobile devices).
                m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, level_9_3_display_attribs);
                if (m_egl_display == EGL_NO_DISPLAY)
                {
                    throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
                }

                if (eglInitialize(m_egl_display, NULL, NULL) == EGL_FALSE)
                {
                    // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is unavailable on the default GPU.
                    m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warp_display_attribs);
                    if (m_egl_display == EGL_NO_DISPLAY)
                    {
                        throw Exception::CreateException(E_FAIL, L"Failed to get EGL display");
                    }

                    if (eglInitialize(m_egl_display, NULL, NULL) == EGL_FALSE)
                    {
                        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
                        throw Exception::CreateException(E_FAIL, L"Failed to initialize EGL");
                    }
                }
            }

            EGLint config_count = 0;
            if ((eglChooseConfig(m_egl_display, config_attribs, &config, 1, &config_count) == EGL_FALSE) || (config_count == 0))
            {
                throw Exception::CreateException(E_FAIL, L"Failed to choose first EGLConfig");
            }

            // Create a PropertySet and initialize with the EGLNativeWindowType.
            PropertySet^ surface_properties = ref new PropertySet();
            surface_properties->Insert(ref new String(EGLNativeWindowTypeProperty), window);

            // You can configure the surface to render at a lower resolution and be scaled up to
            // the full window size. This scaling is often free on mobile hardware.
            //
            // One way to configure the SwapChainPanel is to specify precisely which resolution it should render at.
            // Size customRenderSurfaceSize = Size(800, 600);
            // surfaceCreationProperties->Insert(ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize(customRenderSurfaceSize));
            //
            // Another way is to tell the SwapChainPanel to render at a certain scale factor compared to its size.
            // e.g. if the SwapChainPanel is 1920x1280 then setting a factor of 0.5f will make the app render at 960x640
            // float customResolutionScale = 0.5f;
            // surfaceCreationProperties->Insert(ref new String(EGLRenderResolutionScaleProperty), PropertyValue::CreateSingle(customResolutionScale));

            m_egl_surface = eglCreateWindowSurface(m_egl_display, config, reinterpret_cast<IInspectable*>(surface_properties), surface_attribs);
            if (m_egl_surface == EGL_NO_SURFACE)
            {
                throw Exception::CreateException(E_FAIL, L"Failed to create EGL fullscreen surface");
            }

            m_egl_context = eglCreateContext(m_egl_display, config, EGL_NO_CONTEXT, context_attribs);
            if (m_egl_context == EGL_NO_CONTEXT)
            {
                context_attribs[1] = 2;
                m_egl_context = eglCreateContext(m_egl_display, config, EGL_NO_CONTEXT, context_attribs);
                if (m_egl_context == EGL_NO_CONTEXT)
                {
                    throw Exception::CreateException(E_FAIL, L"Failed to create EGL context");
                }
            }
            else
            {
                m_is_glesv3 = true;
            }

            if (eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface, m_egl_context) == EGL_FALSE)
            {
                throw Exception::CreateException(E_FAIL, L"Failed to make fullscreen EGLSurface current");
            }
        }

        void DoneEGL()
        {
            if (m_egl_display != EGL_NO_DISPLAY && m_egl_surface != EGL_NO_SURFACE)
            {
                eglDestroySurface(m_egl_display, m_egl_surface);
                m_egl_surface = EGL_NO_SURFACE;
            }

            if (m_egl_display != EGL_NO_DISPLAY && m_egl_context != EGL_NO_CONTEXT)
            {
                eglDestroyContext(m_egl_display, m_egl_context);
                m_egl_context = EGL_NO_CONTEXT;
            }

            if (m_egl_display != EGL_NO_DISPLAY)
            {
                eglTerminate(m_egl_display);
                m_egl_display = EGL_NO_DISPLAY;
            }
        }

        // Application lifecycle event handlers.
        void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
        {
            // Run() won't start until the CoreWindow is activated.
            CoreWindow::GetForCurrentThread()->Activate();
        }

        // Window event handlers.
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
        {
            m_window_visible = args->Visible;
        }

        void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args)
        {
            m_window_closed = true;
        }

        void RecreateRenderer()
        {
            this->DoneRenderer();

            EGLint window_width = 0;
            EGLint window_height = 0;
            eglQuerySurface(m_egl_display, m_egl_surface, EGL_WIDTH, &window_width);
            eglQuerySurface(m_egl_display, m_egl_surface, EGL_HEIGHT, &window_height);

            Viry3D::String name = "Viry3D";
            Viry3D::String data_path = ConvertString(Windows::ApplicationModel::Package::Current->InstalledLocation->Path);
            data_path = data_path.Replace("\\", "/") + "/Assets";

            m_display = new Viry3D::Display(name, nullptr, window_width, window_height);
            if (m_is_glesv3)
            {
                m_display->EnableGLESv3();
            }

            m_app = new Viry3D::App();
            m_app->SetName(name);
            m_app->SetDataPath(data_path);
            m_app->Init();
        }

        void DoneRenderer()
        {
            if (m_app)
            {
                delete m_app;
                m_app = nullptr;
            }
            if (m_display)
            {
                delete m_display;
                m_display = nullptr;
            }
        }

        void Draw(int width, int height)
        {
            if (width != m_display->GetWidth() || height != m_display->GetHeight())
            {
                m_display->OnResize(width, height);
            }

            m_app->OnFrameBegin();
            m_app->Update();
            m_display->OnDraw();
            m_app->OnFrameEnd();
        }

        bool m_window_closed = false;
        bool m_window_visible = true;
        EGLDisplay m_egl_display = EGL_NO_DISPLAY;
        EGLContext m_egl_context = EGL_NO_CONTEXT;
        EGLSurface m_egl_surface = EGL_NO_SURFACE;
        bool m_is_glesv3 = false;
        Viry3D::Display* m_display = nullptr;
        Viry3D::App* m_app = nullptr;
    };
}

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class AppSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new app::App();
    }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto app_source = ref new AppSource();
    CoreApplication::Run(app_source);
    return 0;
}
