//
//  AppDelegate.m
//  app
//
//  Created by stack on 2017/7/30.
//  Copyright © 2017年 stack. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

@interface AppDelegate ()

@end

@implementation AppDelegate {
    NSWindow* _window;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    _window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1280, 720) styleMask:
               NSWindowStyleMaskTitled |
               NSWindowStyleMaskClosable |
               NSWindowStyleMaskMiniaturizable |
               NSWindowStyleMaskResizable
        backing:NSBackingStoreBuffered defer:TRUE];
    _window.title = @"Viry3D";
    [_window center];
    [_window makeKeyAndOrderFront:_window];
    _window.delegate = self;
    
    ViewController* viewController = [[ViewController alloc] init];
    viewController._window = _window;
    _window.contentViewController = viewController;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
