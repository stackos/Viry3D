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

#import <Cocoa/Cocoa.h>
#include "Application.h"
#include "Debug.h"
#include "graphics/Graphics.h"
#include "mac/DisplayMac.h"

using namespace Viry3D;

Ref<Viry3D::Application> _app;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property (strong, nonatomic) NSWindow* window;

@end

@implementation AppDelegate;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    _app->OnInit();
    
    self.window = (__bridge_transfer NSWindow*) ((DisplayMac*) Graphics::GetDisplay())->GetWindowBridge();
    self.window.delegate = self;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    ((DisplayMac*) Graphics::GetDisplay())->OnWillResize((int) frameSize.width, (int) frameSize.height);
    return frameSize;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    ((DisplayMac*) Graphics::GetDisplay())->StopRender();
    _app.reset();
    return YES;
}

@end
