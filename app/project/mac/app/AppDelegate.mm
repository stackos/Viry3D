//
//  AppDelegate.m
//  app
//
//  Created by stack on 2017/7/30.
//  Copyright © 2017年 stack. All rights reserved.
//

#import "ViewController.h"
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
