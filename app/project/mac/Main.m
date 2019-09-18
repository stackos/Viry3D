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

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

void initMenu(NSApplication* app) {
    NSMenu* mainMenu = [NSMenu new];
    NSMenuItem* appMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [mainMenu addItem:appMenuItem];
    
    NSMenu* appMenu = [NSMenu new];
    appMenuItem.submenu = appMenu;
    
    NSMenuItem* openVideoItem = [[NSMenuItem alloc] initWithTitle:@"Open Video" action:nil keyEquivalent:@""];
    openVideoItem.target = app.delegate;
    openVideoItem.action = @selector(openVideo);
    
    [appMenu addItem:openVideoItem];
    [appMenu addItem:NSMenuItem.separatorItem];
    [appMenu addItemWithTitle:@"Quit VPaper" action:@selector(terminate:) keyEquivalent:@"q"];
    
    [app setMainMenu:mainMenu];
}

int main(int argc, char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        initMenu(app);
        AppDelegate* delegate = [AppDelegate new];
        app.delegate = delegate;
        [app run];
    }
    return 0;
}
