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

#include "DisplayMac.h"
#include "Application.h"
#include "Debug.h"
#include "Input.h"
#include "graphics/Graphics.h"
#include "time/Time.h"
#import <Cocoa/Cocoa.h>

#if VR_GLES
#import <OpenGL/gl3.h>
#endif

using namespace Viry3D;

extern Ref<Viry3D::Application> _app;

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

static bool g_input_down = false;

#if VR_GLES
@interface OpenGLView : NSOpenGLView
{
    CVDisplayLinkRef displayLink;
    bool stop;
}
@end

@implementation OpenGLView;

- (instancetype)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format
{
    self = [super initWithFrame:frameRect pixelFormat:format];
    NSTrackingArea* area = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                        options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
                                                          owner:self
                                                       userInfo:nil];
    [self addTrackingArea:area];
    return self;
}

- (void)prepareOpenGL
{
    stop = false;
    
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &outputFrame, (__bridge void * _Nullable)(self));
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

static CVReturn outputFrame(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime,
                            CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(__bridge OpenGLView*)displayLinkContext drawFrame:outputTime];
    return result;
}

- (CVReturn)drawFrame:(const CVTimeStamp*)outputTime
{
    if (stop == false)
    {
        auto app = Viry3D::Application::Current();
        auto display = ((Viry3D::DisplayMac*) Viry3D::Graphics::GetDisplay());
        display->DisplayLock();
        
        int width = display->GetTargetWidth();
        int height = display->GetTargetHeight();
        if (width != display->GetWidth() ||
            height != display->GetHeight())
        {
            app->OnResize(width, height);
        }
        
        [[self openGLContext] makeCurrentContext];
        app->OnUpdate();
        app->OnDraw();
        [[self openGLContext] flushBuffer];
        
        display->DisplayUnlock();
    }
    
    return kCVReturnSuccess;
}

- (void)stopRender
{
    stop = true;
    
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
}

- (void)mouseDown:(NSEvent *)event
{
    auto display = ((Viry3D::DisplayMac*) Viry3D::Graphics::GetDisplay());
    display->DisplayLock();
    
    int x = [event locationInWindow].x;
    int y = [event locationInWindow].y;
    
    if (!g_input_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Began;
        t.position = Vector2((float) x, (float) y);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();
        
        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(t);
        }
        else
        {
            g_input_touches.Add(t);
        }
        
        g_input_down = true;
    }
    
    g_mouse_button_down[0] = true;
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) y;
    g_mouse_button_held[0] = true;
    
    display->DisplayUnlock();
}

- (void)mouseUp:(NSEvent *)event
{
    auto display = ((Viry3D::DisplayMac*) Viry3D::Graphics::GetDisplay());
    display->DisplayLock();
    
    int x = [event locationInWindow].x;
    int y = [event locationInWindow].y;
    
    if (g_input_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Ended;
        t.position = Vector2((float) x, (float) y);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();
        
        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(t);
        }
        else
        {
            g_input_touches.Add(t);
        }
        
        g_input_down = false;
    }
    
    g_mouse_button_up[0] = true;
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) y;
    g_mouse_button_held[0] = false;
    
    display->DisplayUnlock();
}

- (void)mouseMoved:(NSEvent *)event
{
    auto display = ((Viry3D::DisplayMac*) Viry3D::Graphics::GetDisplay());
    display->DisplayLock();
    
    int x = [event locationInWindow].x;
    int y = [event locationInWindow].y;
    
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) y;
    
    display->DisplayUnlock();
}

- (void)mouseDragged:(NSEvent *)event
{
    auto display = ((Viry3D::DisplayMac*) Viry3D::Graphics::GetDisplay());
    display->DisplayLock();
    
    int x = [event locationInWindow].x;
    int y = [event locationInWindow].y;
    
    if (g_input_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Moved;
        t.position = Vector2((float) x, (float) y);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();
        
        if (!g_input_touches.Empty())
        {
            if (g_input_touch_buffer.Empty())
            {
                g_input_touch_buffer.AddLast(t);
            }
            else
            {
                if (g_input_touch_buffer.Last().phase == TouchPhase::Moved)
                {
                    g_input_touch_buffer.Last() = t;
                }
                else
                {
                    g_input_touch_buffer.AddLast(t);
                }
            }
        }
        else
        {
            g_input_touches.Add(t);
        }
    }
    
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) y;
    
    display->DisplayUnlock();
}

@end

@interface ViewController : NSViewController
@property (weak, nonatomic) NSWindow* window;
@property (strong, nonatomic) NSOpenGLPixelFormat* pixelFormat;
@end

@implementation ViewController;

- (void)loadView
{
    CGSize size = self.window.contentLayoutRect.size;
    size = [self.window contentRectForFrameRect:self.window.contentLayoutRect].size;
    
    const uint32_t attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        0
    };
    self.pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLView* view = [[OpenGLView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height) pixelFormat:self.pixelFormat];

    auto context = [view openGLContext];
    [context makeCurrentContext];
    
    self.view = view;
}

@end
#endif

namespace Viry3D
{
    
static ViewController* g_view_controller;
static NSOpenGLContext* g_shared_context;
    
void DisplayMac::Init(int width, int height, int fps)
{
    m_target_width = width;
    m_target_height = height;
    
    DisplayBase::Init(width, height, fps);
    
    auto style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    auto frame = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, width, height) styleMask:style];
    
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:TRUE];
    window.title = [NSString stringWithUTF8String:Application::Current()->GetName().CString()];
    [window center];
    [window makeKeyAndOrderFront:window];

    g_view_controller = [[ViewController alloc] init];
    g_view_controller.window = window;
    window.contentViewController = g_view_controller;
    
    m_window = (void*) CFBridgingRetain(window);
}
    
void DisplayMac::Deinit()
{
    g_view_controller = nil;
}
    
void DisplayMac::StopRender()
{
    auto view = (OpenGLView*) g_view_controller.view;
    [view stopRender];
}
    
void* DisplayMac::GetWindowBridge()
{
    return m_window;
}
    
void DisplayMac::BindDefaultFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
    
void DisplayMac::CreateSharedContext()
{
    auto view = (OpenGLView*) g_view_controller.view;
    g_shared_context = [[NSOpenGLContext alloc] initWithFormat:g_view_controller.pixelFormat shareContext:[view openGLContext]];
    [g_shared_context makeCurrentContext];
}

void DisplayMac::DestroySharedContext()
{
    g_shared_context = nil;
}
    
void DisplayMac::OnWillResize(int width, int height)
{
    m_mutex.lock();
    
    auto size = [g_view_controller.window contentRectForFrameRect:NSMakeRect(0, 0, width, height)].size;
    m_target_width = size.width;
    m_target_height = size.height;
    
    m_mutex.unlock();
}
    
void Debug::LogString(const String& str, bool end_line)
{
    NSLog(@"\n%s", str.CString());
}
    
const String& Application::DataPath()
{
    static std::mutex s_mutex;
    
    s_mutex.lock();
    if(m_data_path.Empty())
    {
        String path = [[[NSBundle mainBundle] resourcePath] UTF8String];
        path += "/Assets";
        m_data_path = path;
    }
    s_mutex.unlock();
    
    return m_data_path;
}

const String& Application::SavePath()
{
    static String s_path;
    
    if(s_path.Empty())
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* doc_path = [paths objectAtIndex:0];
        s_path = [doc_path UTF8String];
    }
    
    return s_path;
}
    
}
