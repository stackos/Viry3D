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

#include "DisplayIOS.h"
#include "Application.h"
#include "Debug.h"
#include "graphics/Graphics.h"
#include "graphics/Screen.h"
#include "Input.h"
#import <UIKit/UIKit.h>

#if VR_GLES
#import <GLKit/GLKit.h>
#import <OpenGLES/ES3/gl.h>
#endif

extern Viry3D::Vector<Viry3D::Touch> g_input_touches;
extern Viry3D::List<Viry3D::Touch> g_input_touch_buffer;
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Viry3D::Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];
static Viry3D::Vector<Viry3D::Touch> g_touches;

void touch_begin(void *touches, void *view)
{
    NSSet *set = (__bridge NSSet *) touches;
    UIView *v = (__bridge UIView *) view;
    NSArray *allTouches = [set allObjects];
    int count = (int) [allTouches count];
    int height = [[UIScreen mainScreen] bounds].size.height;
    float scale = [UIScreen mainScreen].nativeScale;
    
    for (int i = 0; i < count; i++)
    {
        UITouch *t = [allTouches objectAtIndex:i];
        CGPoint p = [t locationInView:v];
        p.y = height - p.y - 1;
        
        Viry3D::Touch touch;
        touch.deltaPosition = Viry3D::Vector2(0, 0);
        touch.deltaTime = 0;
        touch.time = t.timestamp;
        touch.fingerId = (int) t.hash;
        touch.phase = (Viry3D::TouchPhase) t.phase;
        touch.tapCount = (int) t.tapCount;
        touch.position = Viry3D::Vector2(p.x, p.y) * scale;
        
        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(touch);
        }
        else
        {
            g_input_touches.Add(touch);
        }
        
        g_touches.Add(touch);
        if (g_touches.Size() == 1)
        {
            g_mouse_button_down[0] = true;
            g_mouse_position.x = touch.position.x;
            g_mouse_position.y = touch.position.y;
            g_mouse_button_held[0] = true;
        }
    }
}

void touch_update(void *touches, void *view)
{
    NSSet *set = (__bridge NSSet *) touches;
    UIView *v = (__bridge UIView *) view;
    NSArray *allTouches = [set allObjects];
    int count = (int) [allTouches count];
    int height = [[UIScreen mainScreen] bounds].size.height;
    float scale = [UIScreen mainScreen].nativeScale;
    
    for (int i = 0; i < count; i++)
    {
        UITouch *t = [allTouches objectAtIndex:i];
        
        CGPoint p = [t locationInView:v];
        p.y = height - p.y - 1;
        
        Viry3D::Touch touch;
        touch.deltaPosition = Viry3D::Vector2(0, 0);
        touch.deltaTime = 0;
        touch.time = t.timestamp;
        touch.fingerId = (int) t.hash;
        touch.phase = (Viry3D::TouchPhase) t.phase;
        touch.tapCount = (int) t.tapCount;
        touch.position = Viry3D::Vector2(p.x, p.y) * scale;
        
        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(touch);
        }
        else
        {
            g_input_touches.Add(touch);
        }
      
        for (int j = 0; j < g_touches.Size(); j++)
        {
            if (touch.fingerId == g_touches[j].fingerId)
            {
                if (touch.phase == Viry3D::TouchPhase::Ended || touch.phase == Viry3D::TouchPhase::Canceled)
                {
                    if (g_touches.Size() == 1)
                    {
                        g_mouse_button_up[0] = true;
                        g_mouse_position.x = touch.position.x;
                        g_mouse_position.y = touch.position.y;
                        g_mouse_button_held[0] = false;
                    }
                    g_touches.Remove(j);
                }
                else if (touch.phase == Viry3D::TouchPhase::Moved)
                {
                    if (g_touches.Size() == 1)
                    {
                        g_mouse_position.x = touch.position.x;
                        g_mouse_position.y = touch.position.y;
                    }
                }
                break;
            }
        }
    }
}

#if VR_GLES
@interface ViewController : GLKViewController
@property (strong, nonatomic) EAGLContext* context;
@end
#endif

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
    
#if VR_GLES
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    GLKView* view = (GLKView*) self.view;
    view.context = self.context;
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;

    [EAGLContext setCurrentContext:self.context];
#endif
    
    self.view.multipleTouchEnabled = true;
}

- (void)orientationDidChange:(NSNotification*)notification
{
    auto bounds = [UIScreen mainScreen].bounds;
    auto scale = [UIScreen mainScreen].nativeScale;
    auto width = bounds.size.width * scale;
    auto height = bounds.size.height * scale;
    auto orientation = [UIDevice currentDevice].orientation;

    if (orientation != UIDeviceOrientationPortraitUpsideDown)
    {
        Viry3D::Screen::SetOrientation((Viry3D::Screen::Orientation) orientation);
        Viry3D::Application::Current()->OnResize(width, height);
    }
}

- (void)dealloc
{
#if VR_GLES
    if ([EAGLContext currentContext] == self.context)
    {
        [EAGLContext setCurrentContext:nil];
    }
#endif
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

#if VR_GLES
- (void)glkView:(GLKView*)view drawInRect:(CGRect)rect
{
    Viry3D::Application::Current()->OnUpdate();
    Viry3D::Application::Current()->OnDraw();
}
#endif

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    if (Viry3D::Application::Current()->IsPaused())
    {
        Viry3D::Application::Current()->OnResume();
    }
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    Viry3D::Application::Current()->OnPause();
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    touch_begin((__bridge void *) touches, (__bridge void *) self.view);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    touch_update((__bridge void *) touches, (__bridge void *) self.view);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    touch_update((__bridge void *) touches, (__bridge void *) self.view);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    touch_update((__bridge void *) touches, (__bridge void *) self.view);
}

@end

namespace Viry3D
{
    
static ViewController* g_view_controller;
static EAGLContext* g_shared_context;
    
void DisplayIOS::Init(int width, int height, int fps)
{
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    
    DisplayBase::Init(bounds.size.width * scale, bounds.size.height * scale, fps);
    
    g_view_controller = [[ViewController alloc] init];
    if (fps <= 0)
    {
        fps = 60;
    }
    g_view_controller.preferredFramesPerSecond = fps;
    
    UIWindow* window = [[UIWindow alloc] initWithFrame:bounds];
    window.rootViewController = g_view_controller;
    [window makeKeyAndVisible];
    
    m_window = (void*) CFBridgingRetain(window);
}
    
void DisplayIOS::Deinit()
{
    g_view_controller = nil;
}
    
void* DisplayIOS::GetWindowBridge()
{
    return m_window;
}
    
void DisplayIOS::BindDefaultFramebuffer()
{
    auto view = (GLKView*) g_view_controller.view;
    [view bindDrawable];
}

int DisplayIOS::GetDefualtDepthRenderBuffer()
{
    this->BindDefaultFramebuffer();
    
    GLint type;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_DEPTH_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &type);
    assert(type == GL_RENDERBUFFER);
    
    GLint depth_texture;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_DEPTH_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                          &depth_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return depth_texture;
}
    
void DisplayIOS::KeepScreenOn(bool enable)
{
    [[UIApplication sharedApplication] setIdleTimerDisabled:enable];
}
    
void DisplayIOS::CreateSharedContext()
{
    g_shared_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:g_view_controller.context.sharegroup];
    auto success = [EAGLContext setCurrentContext:g_shared_context];
    
    Log("DisplayIOS::CreateSharedContext: %lld", (long long) g_shared_context);
    Log("setCurrentContext: %d", success);
}

void DisplayIOS::DestroySharedContext()
{
    Log("DisplayIOS::DestroySharedContext: %lld", (long long) g_shared_context);
    
    g_shared_context = nil;
}

void Debug::LogString(const String& str, bool end_line)
{
    NSLog(@"\n%s", str.CString());
}
    
String Application::DataPath()
{
    static std::mutex s_mutex;
    
    s_mutex.lock();
    if (m_data_path.Empty())
    {
        String path = [[[NSBundle mainBundle] bundlePath] UTF8String];
        path += "/Assets";
        m_data_path = path;
    }
    s_mutex.unlock();
    
    return m_data_path;
}
    
String Application::SavePath()
{
    static String s_path;
    
    if (s_path.Empty())
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* doc_path = [paths objectAtIndex:0];
        s_path = [doc_path UTF8String];
    }
    
    return s_path;
}
    
}
