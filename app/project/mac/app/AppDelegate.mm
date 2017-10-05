/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#import <Cocoa/Cocoa.h>
#include "Application.h"
#include "graphics/Graphics.h"
#include "mac/DisplayMac.h"

using namespace Viry3D;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property (strong, nonatomic) NSWindow *window;

@end

@implementation AppDelegate;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    Application::Current()->OnInit();
    
    self.window = (__bridge_transfer NSWindow*) ((DisplayMac*) Graphics::GetDisplay())->GetWindowBridge();
    
    /*
    NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1280, 720) styleMask:
               NSWindowStyleMaskTitled |
               NSWindowStyleMaskClosable |
               NSWindowStyleMaskMiniaturizable |
               NSWindowStyleMaskResizable
        backing:NSBackingStoreBuffered defer:TRUE];
    window.title = @"Viry3D";
    [window center];
    [_window makeKeyAndOrderFront:window];
    window.delegate = self;
    
    ViewController* viewController = [[ViewController alloc] init];
    viewController._window = window;
    window.contentViewController = viewController;
    
    self.window = window;
     */
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
