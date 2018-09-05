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

#include "ARScene.h"
#include "Debug.h"
#include "math/Matrix4x4.h"
#import <ARKit/ARKit.h>

Viry3D::Matrix4x4 matrix_float4x4_matrix(const matrix_float4x4& mat)
{
    return Viry3D::Matrix4x4(
        mat.columns[0].x, mat.columns[1].x, mat.columns[2].x, mat.columns[3].x,
        mat.columns[0].y, mat.columns[1].y, mat.columns[2].y, mat.columns[3].y,
        mat.columns[0].z, mat.columns[1].z, mat.columns[2].z, mat.columns[3].z,
        mat.columns[0].w, mat.columns[1].w, mat.columns[2].w, mat.columns[3].w);
}

matrix_float4x4 matrix_float4x4_matrix(const Viry3D::Matrix4x4& mat)
{
    matrix_float4x4 matrix;
    matrix.columns[0] = { mat.m00, mat.m10, mat.m20, mat.m30 };
    matrix.columns[1] = { mat.m01, mat.m11, mat.m21, mat.m31 };
    matrix.columns[2] = { mat.m02, mat.m12, mat.m22, mat.m32 };
    matrix.columns[3] = { mat.m03, mat.m13, mat.m23, mat.m33 };
    return matrix;
}

API_AVAILABLE(ios(11.0))
@interface SessionDelegate : NSObject <ARSessionDelegate>

@property (strong, nonatomic) ARSession* session;

@end

@implementation SessionDelegate
{

}

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

- (void)dealloc
{

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

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame
{
    CVPixelBufferRef pixel_buffer = frame.capturedImage;
    if (CVPixelBufferGetPlaneCount(pixel_buffer) < 2)
    {
        return;
    }
    
    [self updateCapturedTexture:pixel_buffer];
}

- (void)updateCapturedTexture:(CVPixelBufferRef)pixel_buffer
{
    CVReturn ret = CVPixelBufferLockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
    if (ret != kCVReturnSuccess)
    {
        return;
    }
    
    int width = (int) CVPixelBufferGetWidthOfPlane(pixel_buffer, 0);
    int height = (int) CVPixelBufferGetHeightOfPlane(pixel_buffer, 0);
    void* pixels = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 0);
    
    width = (int) CVPixelBufferGetWidthOfPlane(pixel_buffer, 1);
    height = (int) CVPixelBufferGetHeightOfPlane(pixel_buffer, 1);
    pixels = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 1);
    
    // y
    // GL_LUMINANCE GL_LUMINANCE GL_UNSIGNED_BYTE
    // uv
    // GL_RG8 GL_RG GL_UNSIGNED_BYTE
    
    CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
}

- (Viry3D::String)addAnchor:(Viry3D::Matrix4x4)transform
{
    ARAnchor* anchor = [[ARAnchor alloc] initWithTransform:matrix_float4x4_matrix(transform)];
    [self.session addAnchor:anchor];
    Viry3D::String id = anchor.identifier.UUIDString.UTF8String;
    return id;
}

- (void)removeAnchor:(const Viry3D::String&)id
{
    NSArray<ARAnchor*>* anchors = self.session.currentFrame.anchors;
    int anchor_count = (int) [anchors count];
    
    for (int i = 0; i < anchor_count; i++)
    {
        ARAnchor* anchor = [anchors objectAtIndex:i];
        if (Viry3D::String(anchor.identifier.UUIDString.UTF8String) == id)
        {
            [self.session removeAnchor:anchor];
            break;
        }
    }
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
        if (@available(iOS 11.0, *))
        {
            return ARConfiguration.isSupported;
        }
        else
        {
            return false;
        }
    }
    
    ARScene::ARScene()
    {
        g_session = [SessionDelegate new];
    }
    
    ARScene::~ARScene()
    {
        g_session = nil;
    }
    
    void ARScene::Run()
    {
        [g_session run];
    }
    
    void ARScene::Pause()
    {
        [g_session pause];
    }
}
