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

#import "AppDelegate.h"
#import "ViewController.h"

@implementation AppDelegate {
    NSWindow* m_window;
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // Insert code here to initialize your application
    
    const char* name = "viry3d-vk-demo";
    int window_width = 1280;
    int window_height = 720;
    
    int style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    NSRect frame = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, window_width, window_height) styleMask:style];
    
    m_window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:TRUE];
    m_window.title = [NSString stringWithUTF8String:name];
    [m_window center];
    [m_window makeKeyAndOrderFront:nil];
    m_window.delegate = self;
    
    ViewController* view_controller = [[ViewController alloc] init];
    view_controller.window = m_window;
    m_window.contentViewController = view_controller;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return TRUE;
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
    // Insert code here to tear down your application
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {
    // resize
    CGSize size = [m_window contentRectForFrameRect:NSMakeRect(0, 0, frameSize.width, frameSize.height)].size;
    float scale = m_window.backingScaleFactor;
    int window_width = size.width * scale;
    int window_height = size.height * scale;
    
    [(ViewController*) m_window.contentViewController onResize:window_width :window_height];
    return frameSize;
}

@end
