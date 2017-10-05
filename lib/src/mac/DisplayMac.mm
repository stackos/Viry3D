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

#include "DisplayMac.h"
#include "Application.h"
#import <Cocoa/Cocoa.h>

#if VR_GLES
#import <OpenGL/gl3.h>
#endif

#if VR_GLES
@interface OpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}
@end

@implementation OpenGLView;

- (void)prepareOpenGL {
    GLint swapInt = 0;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &outputFrame, (__bridge void * _Nullable)(self));
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

static CVReturn outputFrame(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime,
                            CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext) {
    CVReturn result = [(__bridge OpenGLView*)displayLinkContext drawFrame:outputTime];
    return result;
}

- (CVReturn)drawFrame:(const CVTimeStamp*)outputTime {
    [[self openGLContext] makeCurrentContext];
    
    float r = rand() / (float)RAND_MAX;
    float g = rand() / (float)RAND_MAX;
    float b = rand() / (float)RAND_MAX;
    
    glClearColor(r, g, b, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    [[self openGLContext] flushBuffer];
    
    return kCVReturnSuccess;
}

- (void)dealloc {
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
}

@end

@interface ViewController : NSViewController
@property (weak, nonatomic) NSWindow* _window;
@end

@implementation ViewController;

- (void)loadView {
    CGSize size = self._window.contentLayoutRect.size;
    
    const uint32_t attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        0
    };
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLView* view = [[OpenGLView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height) pixelFormat:format];
    
    self.view = view;
}

- (void)viewDidLoad {
    [super viewDidLoad];
}

@end
#endif

namespace Viry3D
{
    
static ViewController* g_view_controller;
static NSOpenGLContext* g_shared_context;
    
void DisplayMac::Init(int width, int height, int fps)
{
    DisplayBase::Init(width, width, fps);
    
    NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:TRUE];
    window.title = @"Viry3D";
    [window center];
    [window makeKeyAndOrderFront:window];

    g_view_controller = [[ViewController alloc] init];
    g_view_controller._window = window;
    window.contentViewController = g_view_controller;
    
    m_window = (void*) CFBridgingRetain(window);
}
    
void DisplayMac::Deinit()
{
    g_view_controller = nil;
}
    
void* DisplayMac::GetWindowBridge()
{
    return m_window;
}
    
void DisplayMac::BindDefaultFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
    
int DisplayMac::GetDefualtDepthRenderBuffer()
{
    return 0;
}
    
void DisplayMac::CreateSharedContext()
{
    
}

void DisplayMac::DestroySharedContext()
{
    
}
    
String Application::DataPath()
{
    static std::mutex s_mutex;
    
    s_mutex.lock();
    if(m_data_path.Empty())
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
    
    if(s_path.Empty())
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* doc_path = [paths objectAtIndex:0];
        s_path = [doc_path UTF8String];
    }
    
    return s_path;
}
    
}
