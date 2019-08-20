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
#include "Engine.h"
#include "Input.h"
#include "container/List.h"

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

@interface View : UIView

@end

@implementation View

+ (Class)layerClass {
#if VR_USE_METAL
    return [CAMetalLayer class];
#else
    return [CAEAGLLayer class];
#endif
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
#if VR_USE_METAL
        CAMetalLayer* layer = (CAMetalLayer*) self.layer;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        
        CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
        layer.drawableSize = nativeBounds.size;
#else
        CAEAGLLayer* layer = (CAEAGLLayer*) self.layer;
        layer.opaque = YES;
#endif
        
        self.contentScaleFactor = UIScreen.mainScreen.nativeScale;
    }

    return self;
}

@end

@interface FrameHandler : NSObject

- (void)setViewController:(ViewController*)vc;

@end

@implementation FrameHandler {
    ViewController* m_vc;
}

- (void)setViewController:(ViewController*)vc {
    m_vc = vc;
}

- (void)drawFrame {
    [m_vc drawFrame];
}

@end

@implementation ViewController {
    CADisplayLink* m_display_link;
    FrameHandler* m_frame_handler;
    Engine* m_engine;
    UIDeviceOrientation m_orientation;
}

- (void)loadView {
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    int window_width = bounds.size.width * scale;
    int window_height = bounds.size.height * scale;
    
    UIView* view = [[View alloc] initWithFrame:CGRectMake(0, 0, window_width, window_height)];
    self.view = view;
    
    m_engine = Engine::Create((__bridge void*) self.view.layer, window_width, window_height);
    
    m_frame_handler = [FrameHandler new];
    [m_frame_handler setViewController:self];
    m_display_link = [CADisplayLink displayLinkWithTarget:m_frame_handler selector:@selector(drawFrame)];
    m_display_link.preferredFramesPerSecond = 0;
    [m_display_link addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
    m_orientation = [UIDevice currentDevice].orientation;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    Engine::Destroy(&m_engine);
    
    [super dealloc];
}

- (void)drawFrame {
    m_engine->Execute();
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
            m_engine->OnResize((__bridge void*) self.view.layer, window_width, window_height);
            
#if VR_USE_METAL
            CAMetalLayer* layer = (CAMetalLayer*) self.view.layer;
            layer.drawableSize = CGSizeMake(window_width, window_height);
#endif
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
