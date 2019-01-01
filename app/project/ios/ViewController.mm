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
#include "App.h"
#include "Input.h"

#if VR_GLES
#import <GLKit/GLKit.h>
#endif

using namespace Viry3D;

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];
static Vector<Touch> g_touches;

static void TouchBegin(NSSet* touches, UIView* view) {
    NSArray* allTouches = [touches allObjects];
    int count = (int) [allTouches count];
    int height = [[UIScreen mainScreen] bounds].size.height;
    float scale = [UIScreen mainScreen].nativeScale;
    
    for (int i = 0; i < count; ++i) {
        UITouch* t = [allTouches objectAtIndex:i];
        CGPoint p = [t locationInView:view];
        p.y = height - p.y - 1;
        
        Touch touch;
        touch.deltaPosition = Vector2(0, 0);
        touch.deltaTime = 0;
        touch.time = t.timestamp;
        touch.fingerId = (int) t.hash;
        touch.phase = (TouchPhase) t.phase;
        touch.tapCount = (int) t.tapCount;
        touch.position = Vector2(p.x, p.y) * scale;
        
        if (!g_input_touches.Empty()) {
            g_input_touch_buffer.AddLast(touch);
        } else {
            g_input_touches.Add(touch);
        }
        
        g_touches.Add(touch);
        if (g_touches.Size() == 1) {
            g_mouse_button_down[0] = true;
            g_mouse_position.x = touch.position.x;
            g_mouse_position.y = touch.position.y;
            g_mouse_button_held[0] = true;
        }
    }
}

static void TouchUpdate(NSSet* touches, UIView* view) {
    NSArray* allTouches = [touches allObjects];
    int count = (int) [allTouches count];
    int height = [[UIScreen mainScreen] bounds].size.height;
    float scale = [UIScreen mainScreen].nativeScale;
    
    for (int i = 0; i < count; ++i) {
        UITouch* t = [allTouches objectAtIndex:i];
        
        CGPoint p = [t locationInView:view];
        p.y = height - p.y - 1;
        
        Touch touch;
        touch.deltaPosition = Vector2(0, 0);
        touch.deltaTime = 0;
        touch.time = t.timestamp;
        touch.fingerId = (int) t.hash;
        touch.phase = (TouchPhase) t.phase;
        touch.tapCount = (int) t.tapCount;
        touch.position = Vector2(p.x, p.y) * scale;
        
        if (t.phase == UITouchPhaseCancelled) {
            touch.phase = TouchPhase::Ended;
        }
        
        if (!g_input_touches.Empty()) {
            g_input_touch_buffer.AddLast(touch);
        } else {
            g_input_touches.Add(touch);
        }
        
        for (int j = 0; j < g_touches.Size(); ++j) {
            if (touch.fingerId == g_touches[j].fingerId) {
                if (touch.phase == TouchPhase::Ended) {
                    if (g_touches.Size() == 1) {
                        g_mouse_button_up[0] = true;
                        g_mouse_position.x = touch.position.x;
                        g_mouse_position.y = touch.position.y;
                        g_mouse_button_held[0] = false;
                    }
                    g_touches.Remove(j);
                } else if (touch.phase == TouchPhase::Moved) {
                    if (g_touches.Size() == 1) {
                        g_mouse_position.x = touch.position.x;
                        g_mouse_position.y = touch.position.y;
                    }
                }
                break;
            }
        }
    }
}

@implementation ViewController {
    CADisplayLink* m_display_link;
    Display* m_display;
    App* m_app;
    UIDeviceOrientation m_orientation;
}

- (void)loadView {
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    int window_width = bounds.size.width * scale;
    int window_height = bounds.size.height * scale;
    
#if VR_VULKAN
    VkView* view = [[VkView alloc] initWithFrame:bounds];
    view.contentScaleFactor = UIScreen.mainScreen.nativeScale;
#elif VR_GLES
    bool glesv3 = false;
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (context == nil) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    } else {
        glesv3 = true;
    }
    GLKView* view = [[GLKView alloc] initWithFrame:bounds context:context];
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormatNone;
    view.drawableMultisample = GLKViewDrawableMultisampleNone;
    view.enableSetNeedsDisplay = FALSE;
    view.delegate = self;
    
    [EAGLContext setCurrentContext:context];
#endif
    
    self.view = view;
    
    String name = "viry3d-vk-demo";
    m_display = new Display(name, (__bridge void*) self.view, window_width, window_height);
    
#if VR_GLES
    m_display->SetBindDefaultFramebufferImplemment([=]() {
        [view bindDrawable];
    });
    if (glesv3) {
        m_display->EnableGLESv3();
    }
#endif
    
    m_app = new App();
    m_app->SetName(name);
	m_app->Init();
    
    m_display_link = [CADisplayLink displayLinkWithTarget: self selector: @selector(drawFrame)];
    [m_display_link setFrameInterval: 1];
    [m_display_link addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
    m_orientation = [UIDevice currentDevice].orientation;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    delete m_app;
    delete m_display;
}

#if VR_GLES
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    [self doDraw];
}
#endif

- (void)drawFrame {
#if VR_VULKAN
    [self doDraw];
#elif VR_GLES
    [((GLKView*) self.view) display];
#endif
}

- (void)doDraw {
    m_app->OnFrameBegin();
    m_app->Update();
    m_display->OnDraw();
    m_app->OnFrameEnd();
}

- (BOOL)prefersStatusBarHidden {
    return TRUE;
}

- (void)orientationDidChange:(NSNotification*)notification {
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    int window_width = bounds.size.width * scale;
    int window_height = bounds.size.height * scale;
    UIDeviceOrientation orientation = [UIDevice currentDevice].orientation;
    
    if (orientation == UIDeviceOrientationPortrait ||
        orientation == UIDeviceOrientationPortraitUpsideDown ||
        orientation == UIDeviceOrientationLandscapeLeft ||
        orientation == UIDeviceOrientationLandscapeRight) {
        if (m_orientation == UIDeviceOrientationUnknown) {
            m_orientation = orientation;
        } else if (m_orientation != orientation) {
            m_orientation = orientation;
            m_display->OnResize(window_width, window_height);
        }
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    TouchBegin(touches, self.view);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    TouchUpdate(touches, self.view);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    TouchUpdate(touches, self.view);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    TouchUpdate(touches, self.view);
}

@end
