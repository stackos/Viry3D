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

#import "VkView.h"
#import <QuartzCore/CAMetalLayer.h>
#include "Input.h"

using namespace Viry3D;

extern bool g_key_down[(int) KeyCode::COUNT];
extern bool g_key[(int) KeyCode::COUNT];
extern bool g_key_up[(int) KeyCode::COUNT];
extern float g_mouse_scroll_wheel;

@implementation View

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    self.contentsScale = 1.0f;
    return self;
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (CALayer*)makeBackingLayer {
    CALayer* layer = [self.class.layerClass layer];
    layer.contentsScale = self.contentsScale;
    return layer;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)onKeyDown:(int)key {
    if (key >= 0) {
        if (!g_key[key]) {
            g_key_down[key] = true;
            g_key[key] = true;
        }
    }
}

- (void)onKeyUp:(int)key {
    if (key >= 0) {
        g_key_up[key] = true;
        g_key[key] = false;
    }
}

- (void)keyDown:(NSEvent*)event {
    
}

- (void)keyUp:(NSEvent*)event {
    
}

- (void)flagsChanged:(NSEvent*)event {
    unsigned int flags = [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    
    bool ctrl = flags & NSEventModifierFlagControl;
    bool shift = flags & NSEventModifierFlagShift;
    bool alt = flags & NSEventModifierFlagOption;
    
    if (ctrl) {
        [self onKeyDown:(int) KeyCode::LeftControl];
    } else {
        if (g_key[(int) KeyCode::LeftControl]) {
            [self onKeyUp:(int) KeyCode::LeftControl];
        }
    }
    
    if (shift) {
        [self onKeyDown:(int) KeyCode::LeftShift];
    } else {
        if (g_key[(int) KeyCode::LeftShift]) {
            [self onKeyUp:(int) KeyCode::LeftShift];
        }
    }
    
    if (alt) {
        [self onKeyDown:(int) KeyCode::LeftAlt];
    } else {
        if (g_key[(int) KeyCode::LeftAlt]) {
            [self onKeyUp:(int) KeyCode::LeftAlt];
        }
    }
}

- (void)scrollWheel:(NSEvent*)event {
    float wheel_dy = 0.0;
    
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
    {
        wheel_dy = [event scrollingDeltaY];
        if ([event hasPreciseScrollingDeltas])
        {
            wheel_dy *= 0.1f;
        }
    }
    else
#endif
    {
        wheel_dy = [event deltaY];
    }
    
    g_mouse_scroll_wheel = wheel_dy * 0.1f;
}

@end
