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

#import "ViewController.h"
#import "VkView.h"
#include "graphics/Display.h"
#include "container/List.h"
#include "thread/ThreadPool.h"
#include "App.h"
#include "Input.h"

using namespace Viry3D;

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

struct MouseEvent
{
    Vector2 position;
    float time;
};

static bool g_mouse_down = false;

@implementation ViewController {
    NSTimer* m_timer;
    Display* m_display;
    App* m_app;
    int m_target_width;
    int m_target_height;
}

- (void)loadView {
    CGSize size = [self.window contentRectForFrameRect:self.window.contentLayoutRect].size;
    float scale = self.window.backingScaleFactor;
    int window_width = size.width * scale;
    int window_height = size.height * scale;
    
#if VR_VULKAN
    View* view = [[View alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
    view.contentsScale = scale;
    view.wantsLayer = YES;
#elif VR_GLES
    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy, // mac gl 3.2 or 4.1 not support glsl 120
        0
    };
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    NSOpenGLView* view = [[NSOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height) pixelFormat:format];
    view.wantsBestResolutionOpenGLSurface = YES;

    // this call will create context
    [view openGLContext];
#endif
    
    self.view = view;
    
    String name = "viry3d-vk-demo";
    m_display = new Display(name, (__bridge void*) self.view, window_width, window_height);
    
    m_app = new App();
    m_app->SetName(name);
    m_app->Init();
    
    m_timer = [NSTimer timerWithTimeInterval:1.0f / 60 target:self selector:@selector(drawFrame) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:m_timer forMode:NSDefaultRunLoopMode];
    
    m_target_width = window_width;
    m_target_height = window_height;
}

- (void)viewWillDisappear {
    [super viewWillDisappear];
    
    [m_timer invalidate];
}

- (void)dealloc {
    delete m_app;
    delete m_display;
}

- (void)onResize:(int)width :(int)height {
    m_target_width = width;
    m_target_height = height;
}

- (void)drawFrame {
    if (m_target_width != m_display->GetWidth() || m_target_height != m_display->GetHeight()) {
        m_display->OnResize(m_target_width, m_target_height);
    }
    
    m_app->OnFrameBegin();
    m_app->Update();
    m_display->OnDraw();
    m_app->OnFrameEnd();
}

- (void)onMouseDown:(const MouseEvent*)e {
    if (!g_mouse_down) {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Began;
        t.position = e->position;
        t.tapCount = 1;
        t.time = e->time;
        
        if (!g_input_touches.Empty()) {
            g_input_touch_buffer.AddLast(t);
        } else {
            g_input_touches.Add(t);
        }
        
        g_mouse_down = true;
    }
    
    g_mouse_button_down[0] = true;
    g_mouse_position.x = e->position.x;
    g_mouse_position.y = e->position.y;
    g_mouse_button_held[0] = true;
}

- (void)onMouseUp:(const MouseEvent*)e {
    if (g_mouse_down) {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Ended;
        t.position = e->position;
        t.tapCount = 1;
        t.time = e->time;
        
        if (!g_input_touches.Empty()) {
            g_input_touch_buffer.AddLast(t);
        } else {
            g_input_touches.Add(t);
        }
        
        g_mouse_down = false;
    }
    
    g_mouse_button_up[0] = true;
    g_mouse_position.x = e->position.x;
    g_mouse_position.y = e->position.y;
    g_mouse_button_held[0] = false;
}

- (void)onMouseMove:(const MouseEvent*)e {
    g_mouse_position.x = e->position.x;
    g_mouse_position.y = e->position.y;
}

- (void)onMouseDrag:(const MouseEvent*)e {
    if (g_mouse_down) {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Moved;
        t.position = e->position;
        t.tapCount = 1;
        t.time = e->time;
        
        if (!g_input_touches.Empty()) {
            if (g_input_touch_buffer.Empty()) {
                g_input_touch_buffer.AddLast(t);
            } else {
                if (g_input_touch_buffer.Last().phase == TouchPhase::Moved) {
                    g_input_touch_buffer.Last() = t;
                } else {
                    g_input_touch_buffer.AddLast(t);
                }
            }
        } else {
            g_input_touches.Add(t);
        }
    }
    
    g_mouse_position.x = e->position.x;
    g_mouse_position.y = e->position.y;
}

- (void)mouseDown:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    [self onMouseDown:&e];
}

- (void)mouseUp:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    [self onMouseUp:&e];
}

- (void)mouseMoved:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    [self onMouseMove:&e];
}

- (void)mouseDragged:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    [self onMouseDrag:&e];
}

@end
