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

-(BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyUp:(NSEvent*)event {
    NSLog(@"keyUp");
}

- (void)keyDown:(NSEvent*)event {
    NSLog(@"keyDown");
}

-(void)flagsChanged:(NSEvent*)event {
    NSLog(@"flagsChanged");
}

-(void)scrollWheel:(NSEvent*)event {
    if (event.type == NSEventTypeScrollWheel) {
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
        
        if (fabs(wheel_dy) > 0.0f) {
            g_mouse_scroll_wheel = wheel_dy * 0.1f;
        }
    }
}

@end
