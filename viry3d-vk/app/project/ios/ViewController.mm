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

@implementation ViewController {
    CADisplayLink* m_display_link;
    Viry3D::Display* m_display;
}

-(void)loadView {
    CGRect bounds = [UIScreen mainScreen].bounds;
    float scale = [UIScreen mainScreen].nativeScale;
    int window_width = bounds.size.width * scale;
    int window_height = bounds.size.height * scale;
    
    self.view = [[VkView alloc] initWithFrame:bounds];
    self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;
    
    m_display = new Viry3D::Display("viry3d-vk-demo", (__bridge void*) self.view, window_width, window_height);
    
    m_display_link = [CADisplayLink displayLinkWithTarget: self selector: @selector(drawFrame)];
    [m_display_link setFrameInterval: 1];
    [m_display_link addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
    // test view
    NSArray* views = [[NSBundle mainBundle] loadNibNamed:@"View" owner:nil options:nil];
    UIView* view = views[0];
    [view setFrame:self.view.frame];
    [self.view addSubview:view];
}

-(void) dealloc {
    delete m_display;
}

-(void)drawFrame {

}

@end
