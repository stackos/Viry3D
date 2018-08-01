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
#include "App.h"
#include "Input.h"

using namespace Viry3D;

Display* g_display;
App* g_app;

static CVReturn DrawFrame(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target) {
    g_app->OnFrameBegin();
    g_app->Update();
    g_display->OnDraw();
    g_app->OnFrameEnd();
    
    return kCVReturnSuccess;
}

@implementation ViewController {
    CVDisplayLinkRef m_display_link;
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
    g_display = new Display(name, (__bridge void*) self.view, window_width, window_height);
    
    g_app = new App();
    g_app->SetName(name);
    g_app->Init();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&m_display_link);
    CVDisplayLinkSetOutputCallback(m_display_link, &DrawFrame, nil);
    CVDisplayLinkStart(m_display_link);
}

-(void) dealloc {
    CVDisplayLinkRelease(m_display_link);
    
    delete g_app;
    delete g_display;
}

@end
