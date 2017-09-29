//
//  GameViewController.m
//  app
//
//  Created by stack on 2017/7/30.
//  Copyright © 2017年 stack. All rights reserved.
//

#import "ViewController.h"
#import <OpenGL/gl3.h>

@interface OpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}
@end

@implementation OpenGLView;

- (void)prepareOpenGL {
    GLint swapInt = 0;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &outputFrame, (__bridge void * _Nullable)(self));
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

static CVReturn outputFrame(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext) {
    CVReturn result = [(__bridge OpenGLView*)displayLinkContext drawFrame:outputTime];
    return result;
}

- (CVReturn)drawFrame:(const CVTimeStamp*)outputTime {
    [[self openGLContext] makeCurrentContext];
    
    float r = rand() / (float)RAND_MAX;
    float g = rand() / (float)RAND_MAX;
    float b = rand() / (float)RAND_MAX;
    
    glClearColor(r, g, b, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    [[self openGLContext] flushBuffer];

    return kCVReturnSuccess;
}

- (void)dealloc {
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
}

@end

@implementation ViewController;

- (void)loadView {
    CGSize size = self._window.contentLayoutRect.size;
    
    const uint32_t attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        0
    };
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLView* view = [[OpenGLView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height) pixelFormat:format];
    
    self.view = view;
}

- (void)viewDidLoad {
    [super viewDidLoad];
} 

@end

