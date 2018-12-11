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
#include "graphics/Texture.h"
#import <ARKit/ARKit.h>

using namespace Viry3D;

static Matrix4x4 matrix_float4x4_matrix(const matrix_float4x4& mat)
{
    return Matrix4x4(
        mat.columns[0].x, mat.columns[1].x, mat.columns[2].x, mat.columns[3].x,
        mat.columns[0].y, mat.columns[1].y, mat.columns[2].y, mat.columns[3].y,
        mat.columns[0].z, mat.columns[1].z, mat.columns[2].z, mat.columns[3].z,
        mat.columns[0].w, mat.columns[1].w, mat.columns[2].w, mat.columns[3].w);
}

static matrix_float4x4 matrix_float4x4_matrix(const Matrix4x4& mat)
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
    Ref<Texture> m_texture_y;
    Ref<Texture> m_texture_uv;
    Ref<Texture> m_texture_env;
    Matrix4x4 m_display_transform;
    Vector3 m_camera_pos;
    Quaternion m_camera_rot;
    float m_camera_fov;
    float m_camera_near;
    float m_camera_far;
    Matrix4x4 m_camera_view_matrix;
    Matrix4x4 m_camera_projection_matrix;
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
    m_texture_y.reset();
    m_texture_uv.reset();
    m_texture_env.reset();
}

- (void)run
{
    ARWorldTrackingConfiguration* config = [ARWorldTrackingConfiguration new];
    config.planeDetection = ARPlaneDetectionHorizontal;
    if (@available(iOS 12, *))
    {
        config.environmentTexturing = AREnvironmentTexturingAutomatic;
    }
    
    [self.session runWithConfiguration:config];
}

- (void)pause
{
    [self.session pause];
}

- (const Ref<Texture>&)getCameraImageY
{
    return m_texture_y;
}

- (const Ref<Texture>&)getCameraImageUV
{
    return m_texture_uv;
}

- (const Ref<Texture>&)getEnvironmentTexture
{
    return m_texture_env;
}

- (const Matrix4x4&)getDisplayTransform
{
    return m_display_transform;
}

- (const Vector3&)getCameraPosition
{
    return m_camera_pos;
}

- (const Quaternion&)getCameraRotation
{
    return m_camera_rot;
}

- (float)getCameraFov
{
    return m_camera_fov;
}

- (float)getCameraNear
{
    return m_camera_near;
}

- (float)getCameraFar
{
    return m_camera_far;
}

- (const Matrix4x4&)getCameraViewMatrix
{
    return m_camera_view_matrix;
}

- (const Matrix4x4&)getCameraProjectionMatrix
{
    return m_camera_projection_matrix;
}

- (void)session:(ARSession*)session didUpdateFrame:(ARFrame*)frame
{
    CVPixelBufferRef pixel_buffer = frame.capturedImage;
    if (CVPixelBufferGetPlaneCount(pixel_buffer) < 2)
    {
        return;
    }
    
    [self updateCapturedTexture:pixel_buffer];
    [self updateDisplayRotation:frame];
    [self updateCamera:frame.camera];
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (int i = 0; i < [anchors count]; ++i)
    {
        ARAnchor* anchor = [anchors objectAtIndex:i];
        if (@available(iOS 12, *))
        {
            if ([anchor isKindOfClass:[AREnvironmentProbeAnchor class]])
            {
                AREnvironmentProbeAnchor* env = (AREnvironmentProbeAnchor*) anchor;
                id<MTLTexture> texture = [env environmentTexture];
                MTLTextureType type = [texture textureType];
                MTLPixelFormat format = [texture pixelFormat];
                int level_count = (int) [texture mipmapLevelCount];
                int width = (int) [texture width];
                int height = (int) [texture height];
                
                if (type != MTLTextureTypeCube)
                {
                    continue;
                }
                
                assert(format == MTLPixelFormatBGRA8Unorm_sRGB);
                assert(width == height);
                
                if (!m_texture_env || m_texture_env->GetWidth() != width || m_texture_env->GetHeight() != height)
                {
                    m_texture_env = Texture::CreateCubemap(width, TextureFormat::R8G8B8A8, FilterMode::Trilinear, SamplerAddressMode::ClampToEdge, true);
                }
                
                ByteBuffer buffer(width * height * 4);
                int w = width;
                int h = height;
                
                for (int j = 0; j < level_count;  ++j)
                {
                    for (int k = 0; k < 6; ++k)
                    {
                        [texture getBytes:buffer.Bytes() bytesPerRow:w * 4 bytesPerImage:w * h * 4 fromRegion:MTLRegionMake2D(0, 0, w, h) mipmapLevel:j slice:k];
                        
                        m_texture_env->UpdateCubemap(buffer, (CubemapFace) k, j);
                    }
                    w = w >> 1;
                    h = h >> 1;
                }
            }
        }
    }
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
    int size = (int) CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 0) * height;
    void* pixels = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 0);
    
    ByteBuffer buffer((byte*) pixels, size);
    if (!m_texture_y || m_texture_y->GetWidth() != width || m_texture_y->GetHeight() != height)
    {
        m_texture_y = Texture::CreateTexture2DFromMemory(
            buffer,
            width, height,
            TextureFormat::R8,
            FilterMode::Linear,
            SamplerAddressMode::ClampToEdge,
            false,
            true,
            false);
    }
    else
    {
        m_texture_y->UpdateTexture2D(buffer, 0, 0, width, height);
    }
    
    width = (int) CVPixelBufferGetWidthOfPlane(pixel_buffer, 1);
    height = (int) CVPixelBufferGetHeightOfPlane(pixel_buffer, 1);
    size = (int) CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 1) * height;
    pixels = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 1);
    
    buffer = ByteBuffer((byte*) pixels, size);
    if (!m_texture_uv || m_texture_uv->GetWidth() != width || m_texture_uv->GetHeight() != height)
    {
        m_texture_uv = Texture::CreateTexture2DFromMemory(
            buffer,
            width, height,
            TextureFormat::R8G8,
            FilterMode::Linear,
            SamplerAddressMode::ClampToEdge,
            false,
            true,
            false);
    }
    else
    {
        m_texture_uv->UpdateTexture2D(buffer, 0, 0, width, height);
    }
    
    CVPixelBufferUnlockBaseAddress(pixel_buffer, kCVPixelBufferLock_ReadOnly);
}

- (UIInterfaceOrientation)orientation
{
    return [[UIApplication sharedApplication] statusBarOrientation];
}

- (CGSize)viewportSize
{
    UIInterfaceOrientation orientation = [self orientation];
    CGSize viewport_size = CGSizeMake(m_texture_y->GetWidth(), m_texture_y->GetHeight());
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        if (m_texture_y->GetWidth() > m_texture_y->GetHeight())
        {
            viewport_size = CGSizeMake(m_texture_y->GetHeight(), m_texture_y->GetWidth());
        }
    }
    else
    {
        if (m_texture_y->GetWidth() < m_texture_y->GetHeight())
        {
            viewport_size = CGSizeMake(m_texture_y->GetHeight(), m_texture_y->GetWidth());
        }
    }
    return viewport_size;
}

- (void)updateDisplayRotation:(ARFrame*)frame
{
    CGAffineTransform transform = [frame displayTransformForOrientation:[self orientation] viewportSize:[self viewportSize]];
    CGPoint a = CGPointMake(0, 0);
    CGPoint b = CGPointMake(1, 0);
    CGPoint a_ = CGPointApplyAffineTransform(a, transform);
    CGPoint b_ = CGPointApplyAffineTransform(b, transform);
    Quaternion rot = Quaternion::FromToRotation(Vector3(b.x - a.x, -(b.y - a.y), 0), Vector3(b_.x - a_.x, -(b_.y - a_.y), 0));
    m_display_transform = Matrix4x4::Rotation(rot);
}

- (void)updateCamera:(ARCamera*)camera
{
    m_camera_near = 0.001f;
    m_camera_far = 100.0f;
    
    UIInterfaceOrientation orientation = [self orientation];
    Matrix4x4 view = matrix_float4x4_matrix([camera viewMatrixForOrientation:orientation]);
    Matrix4x4 proj = matrix_float4x4_matrix([camera projectionMatrixForOrientation:orientation viewportSize:[self viewportSize] zNear:m_camera_near zFar:m_camera_far]);
    Matrix4x4 transform = matrix_float4x4_matrix([camera transform]);
    
    m_camera_pos = transform.MultiplyPoint3x4(Vector3(0, 0, 0));
    Vector3 camera_forward = transform.MultiplyDirection(Vector3(0, 0, -1));
    Vector3 camera_up = transform.MultiplyDirection(Vector3(0, 1, 0));
    m_camera_rot = Quaternion::LookRotation(camera_forward, camera_up);
    m_camera_fov = atan(1.0f / proj.m11) * Mathf::Rad2Deg * 2;
    m_camera_view_matrix = view;
    m_camera_projection_matrix = proj;
}

- (String)addAnchor:(const Matrix4x4&)transform
{
    ARAnchor* anchor = [[ARAnchor alloc] initWithTransform:matrix_float4x4_matrix(transform)];
    [self.session addAnchor:anchor];
    String id = anchor.identifier.UUIDString.UTF8String;
    return id;
}

- (void)removeAnchor:(const String&)id
{
    NSArray<ARAnchor*>* anchors = self.session.currentFrame.anchors;
    int anchor_count = (int) [anchors count];
    
    for (int i = 0; i < anchor_count; i++)
    {
        ARAnchor* anchor = [anchors objectAtIndex:i];
        if (String(anchor.identifier.UUIDString.UTF8String) == id)
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

- (void)sessionWasInterrupted:(ARSession*)session
{
    // Inform the user that the session has been interrupted, for example, by presenting an overlay
    Log("ARSession sessionWasInterrupted");
}

- (void)sessionInterruptionEnded:(ARSession*)session
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
        if (@available(iOS 11, *))
        {
            g_session = [SessionDelegate new];
        }
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
    
    const Ref<Texture>& ARScene::GetCameraTextureY() const
    {
        return [g_session getCameraImageY];
    }
    
    const Ref<Texture>& ARScene::GetCameraTextureUV() const
    {
        return [g_session getCameraImageUV];
    }
    
    const Ref<Texture>& ARScene::GetEnvironmentTexture() const
    {
        return [g_session getEnvironmentTexture];
    }
    
    const Matrix4x4& ARScene::GetDisplayTransform() const
    {
        return [g_session getDisplayTransform];
    }
    
    const Vector3& ARScene::GetCameraPosition() const
    {
        return [g_session getCameraPosition];
    }
    
    const Quaternion& ARScene::GetCameraRotation() const
    {
        return [g_session getCameraRotation];
    }
    
    float ARScene::GetCameraFov() const
    {
        return [g_session getCameraFov];
    }
    
    float ARScene::GetCameraNear() const
    {
        return [g_session getCameraNear];
    }
    
    float ARScene::GetCameraFar() const
    {
        return [g_session getCameraFar];
    }
    
    const Matrix4x4& ARScene::GetCameraViewMatrix() const
    {
        return [g_session getCameraViewMatrix];
    }
    
    const Matrix4x4& ARScene::GetCameraProjectionMatrix() const
    {
        return [g_session getCameraProjectionMatrix];
    }
}
