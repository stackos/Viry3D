//
//  main.m
//  app
//
//  Created by stack on 2017/7/30.
//  Copyright © 2017年 stack. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

int main(int argc, const char * argv[]) {
    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    app.delegate = appDelegate;

    [app run];
    
    return 0;
}
