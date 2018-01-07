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
#include "GameObject.h"
#include "Debug.h"
#include "gles/gles_include.h"
#include "graphics/Camera.h"
#include "graphics/Texture2D.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Screen.h"
#include "renderer/MeshRenderer.h"
#import <ARKit/ARKit.h>

struct CapturedTexture
{
    GLuint texture_y;
    int width_y;
    int height_y;
    GLuint texture_uv;
    int width_uv;
    int height_uv;
};

static const float kImagePlaneVertexData[16] = {
    -1.0, -1.0,  0.0, 1.0,
    1.0, -1.0,  1.0, 1.0,
    -1.0,  1.0,  0.0, 0.0,
    1.0,  1.0,  1.0, 0.0,
};

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
    CVOpenGLESTextureRef m_captured_texture_y;
    CVOpenGLESTextureRef m_captured_texture_uv;
    CVOpenGLESTextureCacheRef m_captured_texture_cache;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        self.session = [ARSession new];
        self.session.delegate = self;
        
        auto context = [EAGLContext currentContext];
        CVOpenGLESTextureCacheCreate(NULL, NULL, context, NULL, &m_captured_texture_cache);
    }
    
    return self;
}

- (void)dealloc {
    CVBufferRelease(m_captured_texture_y);
    CVBufferRelease(m_captured_texture_uv);
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

- (void)update:(CapturedTexture*)texture anchors:(Viry3D::Vector<Viry3D::ARAnchor>&)anchors
{
    ARFrame* frame = self.session.currentFrame;
    memset(texture, 0, sizeof(CapturedTexture));
    
    if (frame == nil)
    {
        return;
    }
    
    CVPixelBufferRef pixel_buffer = frame.capturedImage;
    
    if (CVPixelBufferGetPlaneCount(pixel_buffer) < 2)
    {
        return;
    }
    
    [self _updateCapturedTexture:pixel_buffer texture:texture];
    [self _updateAnchors:frame anchors:anchors];
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
    NSArray<ARAnchor*>* frame_anchors = self.session.currentFrame.anchors;
    int anchor_count = (int) [frame_anchors count];
    
    for (int i = 0; i < anchor_count; i++)
    {
        ARAnchor* anchor = [frame_anchors objectAtIndex:i];
        if (Viry3D::String(anchor.identifier.UUIDString.UTF8String) == id)
        {
            [self.session removeAnchor:anchor];
            break;
        }
    }
}

- (void)_updateCapturedTexture:(CVPixelBufferRef)pixel_buffer texture:(CapturedTexture*)texture
{
    int width_y;
    int height_y;
    int width_uv;
    int height_uv;
    
    CVBufferRelease(m_captured_texture_y);
    CVBufferRelease(m_captured_texture_uv);
    m_captured_texture_y = [self _createTextureFromPixelBuffer:pixel_buffer internalFormat:GL_LUMINANCE format:GL_LUMINANCE type:GL_UNSIGNED_BYTE planeIndex:0 width:&width_y height:&height_y];
    m_captured_texture_uv = [self _createTextureFromPixelBuffer:pixel_buffer internalFormat:GL_RG8 format:GL_RG type:GL_UNSIGNED_BYTE planeIndex:1 width:&width_uv height:&height_uv];
    
    if (m_captured_texture_y != nil &&
        m_captured_texture_uv != nil)
    {
        texture->texture_y = CVOpenGLESTextureGetName(m_captured_texture_y);
        texture->width_y = width_y;
        texture->height_y = height_y;
        texture->texture_uv = CVOpenGLESTextureGetName(m_captured_texture_uv);
        texture->width_uv = width_uv;
        texture->height_uv = height_uv;
    }
}

- (CVOpenGLESTextureRef)_createTextureFromPixelBuffer:(CVPixelBufferRef)pixel_buffer internalFormat:(GLint)internal_format format:(GLint)format type:(GLenum)type planeIndex:(NSInteger)plane_index width:(int*)width height:(int*)height {
    *width = (int) CVPixelBufferGetWidthOfPlane(pixel_buffer, plane_index);
    *height = (int) CVPixelBufferGetHeightOfPlane(pixel_buffer, plane_index);
    
    CVOpenGLESTextureRef texture = nil;
    CVReturn status = CVOpenGLESTextureCacheCreateTextureFromImage(NULL, m_captured_texture_cache, pixel_buffer, NULL, GL_TEXTURE_2D, internal_format, *width, *height, format, type, plane_index, &texture);
    if (status != kCVReturnSuccess)
    {
        CVBufferRelease(texture);
        texture = nil;
    }
    
    return texture;
}

- (void)_updateAnchors:(ARFrame*)frame anchors:(Viry3D::Vector<Viry3D::ARAnchor>&)anchors
{
    NSArray<ARAnchor*>* frame_anchors = frame.anchors;
    int anchor_count = (int) [frame_anchors count];
    
    anchors.Clear();
    
    for (int i = 0; i < anchor_count; i++)
    {
        ARAnchor* anchor = [frame_anchors objectAtIndex:i];
        if ([anchor isKindOfClass:[ARPlaneAnchor class]])
        {
            ARPlaneAnchor* plane = (ARPlaneAnchor*) anchor;

            Viry3D::ARAnchor a;
            a.id = plane.identifier.UUIDString.UTF8String;
            a.transform = matrix_float4x4_matrix(plane.transform);
            a.is_plane = true;
            a.center = Viry3D::Vector3(plane.center.x, plane.center.y, plane.center.z);
            a.extent = Viry3D::Vector3(plane.extent.x, plane.extent.y, plane.extent.z);
            
            anchors.Add(a);
        }
        else
        {
            Viry3D::ARAnchor a;
            a.id = anchor.identifier.UUIDString.UTF8String;
            a.transform = matrix_float4x4_matrix(anchor.transform);
            a.is_plane = false;
            a.center = Viry3D::Vector3::Zero();
            a.extent = Viry3D::Vector3::Zero();
            
            anchors.Add(a);
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
        return ARConfiguration.isSupported;
    }
    
    ARScene::ARScene(int camera_depth, int camera_culling_mask, int bg_layer):
        m_resized(true)
    {
        g_session = [[SessionDelegate alloc] init];
        
        m_camera = GameObject::Create("camera")->AddComponent<Camera>();
        m_camera->SetDepth(camera_depth);
        m_camera->SetCullingMask(camera_culling_mask);
        m_camera->SetFrustumCulling(false);
        
        auto mesh = Mesh::Create();
        for (int i = 0; i < 4; i++)
        {
            mesh->vertices.Add(Vector3(kImagePlaneVertexData[i * 4], kImagePlaneVertexData[i * 4 + 1], 0));
            mesh->uv.Add(Vector2(kImagePlaneVertexData[i * 4 + 2], kImagePlaneVertexData[i * 4 + 3]));
        }
        unsigned short triangles[] = {
            0, 1, 2, 2, 1, 3
        };
        mesh->triangles.AddRange(triangles, 6);
        mesh->Apply();
        
        auto mat = Material::Create("YUVToRGB");
        
        auto obj = GameObject::Create("mesh");
        obj->SetLayerRecursively(bg_layer);
        obj->SetActive(false);
        
        auto renderer = obj->AddComponent<MeshRenderer>();
        renderer->SetSharedMesh(mesh);
        renderer->SetSharedMaterial(mat);
        
        m_background_mat = mat;
        m_background_obj = obj;
        m_background_mesh = mesh;
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
            CapturedTexture background;
            
            [g_session update:&background anchors:m_anchors];
            
            if (background.texture_y > 0 &&
                background.texture_uv > 0)
            {
                if (m_background_texture_y)
                {
                    m_background_texture_y->UpdateExternalTexture((void*) (size_t) background.texture_y);
                }
                else
                {
                    m_background_texture_y = Texture2D::CreateExternalTexture(background.width_y, background.height_y, TextureFormat::R8, false, (void*) (size_t) background.texture_y);
                }
                
                if (m_background_texture_uv)
                {
                    m_background_texture_uv->UpdateExternalTexture((void*) (size_t) background.texture_uv);
                }
                else
                {
                     m_background_texture_uv = Texture2D::CreateExternalTexture(background.width_uv, background.height_uv, TextureFormat::RG16, false, (void*) (size_t) background.texture_uv);
                }
                
                auto texture_y = GetBackgroundTextureY();
                auto texture_uv = GetBackgroundTextureUV();
                if (texture_y && texture_uv)
                {
                    if (m_background_mat && m_background_obj)
                    {
                        if (m_background_obj->IsActiveSelf() == false)
                        {
                            m_background_obj->SetActive(true);
                        }
                        
                        m_background_mat->SetTexture("_MainTexY", texture_y);
                        m_background_mat->SetTexture("_MainTexUV", texture_uv);
                    };
                }
                
                auto frame = g_session.session.currentFrame;
                auto screen_size = CGSizeMake(Screen::GetWidth(), Screen::GetHeight());
                auto orientation = (UIInterfaceOrientation) Screen::GetOrientation();
                
                if (m_resized)
                {
                    m_resized = false;
                    
                    CGAffineTransform uv_transform = CGAffineTransformInvert([frame displayTransformForOrientation:orientation viewportSize:screen_size]);
                    
                    auto mesh = m_background_mesh;
                    mesh->vertices.Clear();
                    mesh->uv.Clear();
                    for (int i = 0; i < 4; i++)
                    {
                        mesh->vertices.Add(Vector3(kImagePlaneVertexData[i * 4], kImagePlaneVertexData[i * 4 + 1], 0));
                        CGPoint uv = CGPointMake(kImagePlaneVertexData[i * 4 + 2], kImagePlaneVertexData[i * 4 + 3]);
                        CGPoint uv_transformed = CGPointApplyAffineTransform(uv, uv_transform);
                        mesh->uv.Add(Vector2(uv_transformed.x, uv_transformed.y));
                    }
                    mesh->Apply();
                }
                
                m_camera_view_matrix = matrix_float4x4_matrix([frame.camera viewMatrixForOrientation:orientation]);
                m_camera_projection_matrix = matrix_float4x4_matrix([frame.camera projectionMatrixForOrientation:orientation viewportSize:screen_size zNear:0.03f zFar:100.0f]);
                m_camera_transform = matrix_float4x4_matrix([frame.camera transform]);
                
                m_camera->SetViewMatrixExternal(m_camera_view_matrix);
                m_camera->SetProjectionMatrixExternal(m_camera_projection_matrix);
                m_camera->GetTransform()->SetLocalToWorldMatrixExternal(m_camera_transform);
            }
        }
    }
    
    void ARScene::OnResize(int width, int height)
    {
        m_resized = true;
    }
    
    String ARScene::AddAnchor(const Matrix4x4& transform)
    {
        if (g_session != nil)
        {
            return [g_session addAnchor:transform];
        }
        
        return "";
    }
    
    void ARScene::RemoveAnchor(const String& id)
    {
        if (g_session != nil)
        {
            [g_session removeAnchor:id];
        }
    }
}
