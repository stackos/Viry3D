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

enum class MouseEventType
{
    Down,
    Up,
    Move,
    Drag,
};

struct MouseEvent
{
    MouseEventType type;
    Vector2 position;
    float time;
};

static bool g_mouse_down = false;
static Vector<MouseEvent> g_events;
static Mutex g_mutex;

@implementation ViewController {
    CVDisplayLinkRef m_display_link;
    Display* m_display;
    App* m_app;
}

static CVReturn DrawFrame(CVDisplayLinkRef displayLink,
                          const CVTimeStamp* now,
                          const CVTimeStamp* outputTime,
                          CVOptionFlags flagsIn,
                          CVOptionFlags* flagsOut,
                          void* target) {
    [(__bridge ViewController*) target drawFrame];
    return kCVReturnSuccess;
}

-(void)loadView {
    CGSize size = self.window.contentLayoutRect.size;
    size = [self.window contentRectForFrameRect:self.window.contentLayoutRect].size;
    float scale = self.window.backingScaleFactor;
    int window_width = size.width * scale;
    int window_height = size.height * scale;
    
    VkView* view = [[VkView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
    view.wantsLayer = YES;
    self.view = view;
    
    String name = "viry3d-vk-demo";
    m_display = new Display(name, (__bridge void*) self.view, window_width, window_height);
    
    m_app = new App();
    m_app->SetName(name);
    m_app->Init();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&m_display_link);
    CVDisplayLinkSetOutputCallback(m_display_link, &DrawFrame, (__bridge void*) self);
    CVDisplayLinkStart(m_display_link);
}

-(void)dealloc {
    CVDisplayLinkRelease(m_display_link);
    
    delete m_app;
    delete m_display;
}

-(void)drawFrame {
    [self processEvents];
    
    m_app->OnFrameBegin();
    m_app->Update();
    m_display->OnDraw();
    m_app->OnFrameEnd();
}

-(void)processEvents {
    g_mutex.lock();
    for (const auto& i : g_events) {
        switch (i.type) {
            case MouseEventType::Down:
                [self onMouseDown:&i];
                break;
            case MouseEventType::Up:
                [self onMouseUp:&i];
                break;
            case MouseEventType::Move:
                [self onMouseMove:&i];
                break;
            case MouseEventType::Drag:
                [self onMouseDrag:&i];
                break;
        }
    }
    g_events.Clear();
    g_mutex.unlock();
}

-(void)onMouseDown:(const MouseEvent*)e {
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

-(void)onMouseUp:(const MouseEvent*)e {
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

-(void)onMouseMove:(const MouseEvent*)e {
    g_mouse_position.x = e->position.x;
    g_mouse_position.y = e->position.y;
}

-(void)onMouseDrag:(const MouseEvent*)e {
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

-(void)mouseDown:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.type = MouseEventType::Down;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    g_mutex.lock();
    g_events.Add(e);
    g_mutex.unlock();
}

-(void)mouseUp:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.type = MouseEventType::Up;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    g_mutex.lock();
    g_events.Add(e);
    g_mutex.unlock();
}

-(void)mouseMoved:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.type = MouseEventType::Move;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    g_mutex.lock();
    g_events.Add(e);
    g_mutex.unlock();
}

-(void)mouseDragged:(NSEvent*)event {
    float scale = self.window.backingScaleFactor;
    float x = [event locationInWindow].x * scale;
    float y = [event locationInWindow].y * scale;
    
    MouseEvent e;
    e.type = MouseEventType::Drag;
    e.position = Vector2(x, y);
    e.time = [event timestamp];
    
    g_mutex.lock();
    g_events.Add(e);
    g_mutex.unlock();
}

@end
