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

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    // Insert code here to initialize your application
    
    const char* name = "Viry3D";
    int window_width = 1280;
    int window_height = 720;
    
    int style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    NSRect frame = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, window_width, window_height) styleMask:style];
    
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:TRUE];
    window.title = [NSString stringWithUTF8String:name];
    [window center];
    [window makeKeyAndOrderFront:nil];
    window.delegate = self;
    
    ViewController* view_controller = [[ViewController alloc] init];
    [view_controller setWindow:window];
    window.contentViewController = view_controller;
    [view_controller release];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return TRUE;
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
    // Insert code here to tear down your application
}

- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)frameSize {
    // resize
    CGSize size = [window contentRectForFrameRect:NSMakeRect(0, 0, frameSize.width, frameSize.height)].size;
    float scale = window.backingScaleFactor;
    int window_width = size.width * scale;
    int window_height = size.height * scale;
    
    [(ViewController*) window.contentViewController onResize:window_width :window_height];
    return frameSize;
}

@end
