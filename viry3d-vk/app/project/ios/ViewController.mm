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
#include "graphics/Camera.h"
#include "App.h"

using namespace Viry3D;

@implementation ViewController {
    CADisplayLink* m_display_link;
    Display* m_display;
    App* m_app;
    UIDeviceOrientation m_orientation;
}

-(void)loadView {
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    int window_width = bounds.size.width * scale;
    int window_height = bounds.size.height * scale;
    
    self.view = [[VkView alloc] initWithFrame:bounds];
    self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;
    
    String name = "viry3d-vk-demo";
    m_display = new Display(name, (__bridge void*) self.view, window_width, window_height);
    
    m_app = new App();
    m_app->SetName(name);
    
    m_display_link = [CADisplayLink displayLinkWithTarget: self selector: @selector(drawFrame)];
    [m_display_link setFrameInterval: 1];
    [m_display_link addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
    m_orientation = [UIDevice currentDevice].orientation;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
    
    // test view
    NSArray* views = [[NSBundle mainBundle] loadNibNamed:@"View" owner:nil options:nil];
    UIView* view = views[0];
    [view setFrame:self.view.frame];
    [self.view addSubview:view];
}

-(void) dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    delete m_app;
    delete m_display;
}

-(void)drawFrame {
    m_app->OnFrameBegin();
    m_app->Update();
    m_display->OnDraw();
    m_app->OnFrameEnd();
}

-(BOOL)prefersStatusBarHidden {
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

@end
