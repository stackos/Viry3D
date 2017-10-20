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

#include "ARScene.h"
#include "Debug.h"
#import <ARKit/ARKit.h>

API_AVAILABLE(ios(11.0))
@interface SessionDelegate : NSObject <ARSessionDelegate>

@property (nonatomic, strong) ARSession* session;

@end

@implementation SessionDelegate

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.session = [ARSession new];
        self.session.delegate = self;
    }
    
    return self;
}

- (void)run
{
    ARWorldTrackingConfiguration* configuration = [ARWorldTrackingConfiguration new];
    configuration.planeDetection = ARPlaneDetectionHorizontal;
    
    [self.session runWithConfiguration:configuration];
}

- (void)pause
{
    [self.session pause];
}

- (void)update
{
    ARFrame* frame = _session.currentFrame;
    
    if (frame == nil)
    {
        return;
    }
    
    Log("get ar frame!!!");
}

- (void)session:(ARSession *)session didFailWithError:(NSError *)error
{
    Log("ARSession didFailWithError:%s", [[error localizedDescription] UTF8String]);
}

- (void)sessionWasInterrupted:(ARSession *)session
{
    // Inform the user that the session has been interrupted, for example, by presenting an overlay
    Log("ARSession sessionWasInterrupted");
}

- (void)sessionInterruptionEnded:(ARSession *)session
{
    // Reset tracking and/or remove existing anchors if consistent tracking is required
    Log("ARSession sessionInterruptionEnded");
}

@end

namespace Viry3D
{
    static SessionDelegate* g_session = nil;
    
	bool ARScene::IsSupported()
    {
        return ARConfiguration.isSupported;
    }
    
    ARScene::ARScene()
    {
        g_session = [[SessionDelegate alloc] init];
    }
    
    ARScene::~ARScene()
    {
        g_session = nil;
    }
    
    void ARScene::RunSession()
    {
        if (g_session != nil)
        {
            [g_session run];
        }
    }
    
    void ARScene::PauseSession()
    {
        if (g_session != nil)
        {
            [g_session pause];
        }
    }
    
    void ARScene::UpdateSession()
    {
        if (g_session != nil)
        {
            [g_session update];
        }
    }
}
