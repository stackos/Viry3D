#version 300 es
#define WEBGL2 
#define PBR
#define MAINUV1
#define UV1
#define ALBEDO
#define ALBEDODIRECTUV 1
#define AMBIENT
#define AMBIENTDIRECTUV 1
#define AMBIENTINGRAYSCALE
#define OPACITYDIRECTUV 0
#define ALPHATESTVALUE 0.4
#define SPECULAROVERALPHA
#define RADIANCEOVERALPHA
#define EMISSIVE
#define EMISSIVEDIRECTUV 1
#define REFLECTIVITY
#define REFLECTIVITYDIRECTUV 1
#define LODBASEDMICROSFURACE
#define MICROSURFACEMAPDIRECTUV 0
#define METALLICWORKFLOW
#define ROUGHNESSSTOREINMETALMAPGREEN
#define METALLNESSSTOREINMETALMAPBLUE
#define ENVIRONMENTBRDF
#define NORMAL
#define BUMP
#define BUMPDIRECTUV 1
#define NORMALXYSCALE
#define LIGHTMAPDIRECTUV 0
#define REFLECTION
#define REFLECTIONMAP_3D
#define REFLECTIONMAP_CUBIC
#define USESPHERICALFROMREFLECTIONMAP
#define USESPHERICALINVERTEX
#define RADIANCEOCCLUSION
#define HORIZONOCCLUSION
#define NUM_BONE_INFLUENCERS 0
#define BonesPerMesh 0
#define NUM_MORPH_INFLUENCERS 0
#define IMAGEPROCESSING
#define VIGNETTEBLENDMODEMULTIPLY
#define TONEMAPPING
#define CONTRAST
#define SAMPLER3DGREENDEPTH
#define SAMPLER3DBGRMAP
#define EXPOSURE
#define USEPHYSICALLIGHTFALLOFF
#define SPECULARAA

#define SHADER_NAME vertex:pbr
precision highp float;
layout(std140,column_major) uniform;
uniform Material
{
uniform vec2 vAlbedoInfos;
uniform vec4 vAmbientInfos;
uniform vec2 vOpacityInfos;
uniform vec2 vEmissiveInfos;
uniform vec2 vLightmapInfos;
uniform vec3 vReflectivityInfos;
uniform vec2 vMicroSurfaceSamplerInfos;
uniform vec4 vRefractionInfos;
uniform vec2 vReflectionInfos;
uniform vec3 vReflectionPosition;
uniform vec3 vReflectionSize; 
uniform vec3 vBumpInfos;
uniform mat4 albedoMatrix;
uniform mat4 ambientMatrix;
uniform mat4 opacityMatrix;
uniform mat4 emissiveMatrix;
uniform mat4 lightmapMatrix;
uniform mat4 reflectivityMatrix;
uniform mat4 microSurfaceSamplerMatrix;
uniform mat4 bumpMatrix;
uniform vec2 vTangentSpaceParams;
uniform mat4 refractionMatrix;
uniform mat4 reflectionMatrix;
uniform vec3 vReflectionColor;
uniform vec4 vAlbedoColor;
uniform vec4 vLightingIntensity;
uniform vec3 vRefractionMicrosurfaceInfos;
uniform vec3 vReflectionMicrosurfaceInfos;
uniform vec4 vReflectivityColor;
uniform vec3 vEmissiveColor;
uniform float pointSize;
};
uniform Scene {
mat4 viewProjection;
mat4 view;
};

in vec3 position;
#ifdef NORMAL
in vec3 normal;
#endif
#ifdef TANGENT
in vec4 tangent;
#endif
#ifdef UV1
in vec2 uv;
#endif
#ifdef UV2
in vec2 uv2;
#endif
#ifdef MAINUV1
out vec2 vMainUV1;
#endif
#ifdef MAINUV2
out vec2 vMainUV2; 
#endif 
#ifdef VERTEXCOLOR
in vec4 color;
#endif
const float PI=3.1415926535897932384626433832795;
const float LinearEncodePowerApprox=2.2;
const float GammaEncodePowerApprox=1.0/LinearEncodePowerApprox;
const vec3 LuminanceEncodeApprox=vec3(0.2126,0.7152,0.0722);
mat3 transposeMat3(mat3 inMatrix) {
vec3 i0=inMatrix[0];
vec3 i1=inMatrix[1];
vec3 i2=inMatrix[2];
mat3 outMatrix=mat3(
vec3(i0.x,i1.x,i2.x),
vec3(i0.y,i1.y,i2.y),
vec3(i0.z,i1.z,i2.z)
);
return outMatrix;
}

mat3 inverseMat3(mat3 inMatrix) {
float a00=inMatrix[0][0],a01=inMatrix[0][1],a02=inMatrix[0][2];
float a10=inMatrix[1][0],a11=inMatrix[1][1],a12=inMatrix[1][2];
float a20=inMatrix[2][0],a21=inMatrix[2][1],a22=inMatrix[2][2];
float b01=a22*a11-a12*a21;
float b11=-a22*a10+a12*a20;
float b21=a21*a10-a11*a20;
float det=a00*b01+a01*b11+a02*b21;
return mat3(b01,(-a22*a01+a02*a21),(a12*a01-a02*a11),
b11,(a22*a00-a02*a20),(-a12*a00+a02*a10),
b21,(-a21*a00+a01*a20),(a11*a00-a01*a10))/det;
}
float computeFallOff(float value,vec2 clipSpace,float frustumEdgeFalloff)
{
float mask=smoothstep(1.0-frustumEdgeFalloff,1.0,clamp(dot(clipSpace,clipSpace),0.,1.));
return mix(value,1.0,mask);
}
vec3 applyEaseInOut(vec3 x){
return x*x*(3.0-2.0*x);
}
vec3 toLinearSpace(vec3 color)
{
return pow(color,vec3(LinearEncodePowerApprox));
}
vec3 toGammaSpace(vec3 color)
{
return pow(color,vec3(GammaEncodePowerApprox));
}
float square(float value)
{
return value*value;
}
float getLuminance(vec3 color)
{
return clamp(dot(color,LuminanceEncodeApprox),0.,1.);
}

float getRand(vec2 seed) {
return fract(sin(dot(seed.xy ,vec2(12.9898,78.233)))*43758.5453);
}
float dither(vec2 seed,float varianceAmount) {
float rand=getRand(seed);
float dither=mix(-varianceAmount/255.0,varianceAmount/255.0,rand);
return dither;
}

const float rgbdMaxRange=255.0;
vec4 toRGBD(vec3 color) {
float maxRGB=max(0.0000001,max(color.r,max(color.g,color.b)));
float D=max(rgbdMaxRange/maxRGB,1.);
D=clamp(floor(D)/255.0,0.,1.);

vec3 rgb=color.rgb*D;

rgb=toGammaSpace(rgb);
return vec4(rgb,D); 
}
vec3 fromRGBD(vec4 rgbd) {

rgbd.rgb=toLinearSpace(rgbd.rgb);

return rgbd.rgb/rgbd.a;
}
#if NUM_BONE_INFLUENCERS>0
uniform mat4 mBones[BonesPerMesh];
in vec4 matricesIndices;
in vec4 matricesWeights;
#if NUM_BONE_INFLUENCERS>4
in vec4 matricesIndicesExtra;
in vec4 matricesWeightsExtra;
#endif
#endif

#ifdef INSTANCES
in vec4 world0;
in vec4 world1;
in vec4 world2;
in vec4 world3;
#else
uniform mat4 world;
#endif
#if defined(ALBEDO) && ALBEDODIRECTUV == 0
out vec2 vAlbedoUV;
#endif
#if defined(AMBIENT) && AMBIENTDIRECTUV == 0
out vec2 vAmbientUV;
#endif
#if defined(OPACITY) && OPACITYDIRECTUV == 0
out vec2 vOpacityUV;
#endif
#if defined(EMISSIVE) && EMISSIVEDIRECTUV == 0
out vec2 vEmissiveUV;
#endif
#if defined(LIGHTMAP) && LIGHTMAPDIRECTUV == 0
out vec2 vLightmapUV;
#endif
#if defined(REFLECTIVITY) && REFLECTIVITYDIRECTUV == 0
out vec2 vReflectivityUV;
#endif
#if defined(MICROSURFACEMAP) && MICROSURFACEMAPDIRECTUV == 0
out vec2 vMicroSurfaceSamplerUV;
#endif
#if defined(BUMP) && BUMPDIRECTUV == 0
out vec2 vBumpUV;
#endif

out vec3 vPositionW;
#ifdef NORMAL
out vec3 vNormalW;
#if defined(USESPHERICALFROMREFLECTIONMAP) && defined(USESPHERICALINVERTEX)
out vec3 vEnvironmentIrradiance;
#ifdef USESPHERICALFROMREFLECTIONMAP
uniform vec3 vSphericalX;
uniform vec3 vSphericalY;
uniform vec3 vSphericalZ;
uniform vec3 vSphericalXX_ZZ;
uniform vec3 vSphericalYY_ZZ;
uniform vec3 vSphericalZZ;
uniform vec3 vSphericalXY;
uniform vec3 vSphericalYZ;
uniform vec3 vSphericalZX;
vec3 quaternionVectorRotation_ScaledSqrtTwo(vec4 Q,vec3 V){
vec3 T=cross(Q.xyz,V);
T+=Q.www*V;
return cross(Q.xyz,T)+V;
}
vec3 environmentIrradianceJones(vec3 normal)
{









float Nx=normal.x;
float Ny=normal.y;
float Nz=normal.z;
vec3 C1=vSphericalZZ.rgb;
vec3 Cx=vSphericalX.rgb;
vec3 Cy=vSphericalY.rgb;
vec3 Cz=vSphericalZ.rgb;
vec3 Cxx_zz=vSphericalXX_ZZ.rgb;
vec3 Cyy_zz=vSphericalYY_ZZ.rgb;
vec3 Cxy=vSphericalXY.rgb;
vec3 Cyz=vSphericalYZ.rgb;
vec3 Czx=vSphericalZX.rgb;
vec3 a1=Cyy_zz*Ny+Cy;
vec3 a2=Cyz*Nz+a1;
vec3 b1=Czx*Nz+Cx;
vec3 b2=Cxy*Ny+b1;
vec3 b3=Cxx_zz*Nx+b2;
vec3 t1=Cz*Nz+C1;
vec3 t2=a2*Ny+t1;
vec3 t3=b3*Nx+t2;
return t3;
}
#endif
#endif
#endif
#ifdef VERTEXCOLOR
out vec4 vColor;
#endif
#if defined(BUMP) || defined(PARALLAX)
#if defined(TANGENT) && defined(NORMAL) 
out mat3 vTBN;
#endif
#endif

#ifdef CLIPPLANE
uniform vec4 vClipPlane;
out float fClipDistance;
#endif
#ifdef CLIPPLANE2
uniform vec4 vClipPlane2;
out float fClipDistance2;
#endif
#ifdef CLIPPLANE3
uniform vec4 vClipPlane3;
out float fClipDistance3;
#endif
#ifdef CLIPPLANE4
uniform vec4 vClipPlane4;
out float fClipDistance4;
#endif
#ifdef FOG
out vec3 vFogDistance;
#endif
#ifdef LIGHT0
uniform Light0
{
vec4 vLightData;
vec4 vLightDiffuse;
vec3 vLightSpecular;
#ifdef SPOTLIGHT0
vec4 vLightDirection;
vec4 vLightFalloff;
#elif defined(POINTLIGHT0)
vec4 vLightFalloff;
#elif defined(HEMILIGHT0)
vec3 vLightGround;
#endif
vec4 shadowsInfo;
vec2 depthValues;
} light0;
#ifdef PROJECTEDLIGHTTEXTURE0
uniform mat4 textureProjectionMatrix0;
uniform sampler2D projectionLightSampler0;
#endif
#ifdef SHADOW0
#if defined(SHADOWCUBE0)
uniform samplerCube shadowSampler0; 
#else
out vec4 vPositionFromLight0;
out float vDepthMetric0;
#if defined(SHADOWPCSS0)
uniform highp sampler2DShadow shadowSampler0;
uniform highp sampler2D depthSampler0;
#elif defined(SHADOWPCF0)
uniform highp sampler2DShadow shadowSampler0;
#else
uniform sampler2D shadowSampler0;
#endif
uniform mat4 lightMatrix0;
#endif
#endif
#endif
#ifdef LIGHT1
uniform Light1
{
vec4 vLightData;
vec4 vLightDiffuse;
vec3 vLightSpecular;
#ifdef SPOTLIGHT1
vec4 vLightDirection;
vec4 vLightFalloff;
#elif defined(POINTLIGHT1)
vec4 vLightFalloff;
#elif defined(HEMILIGHT1)
vec3 vLightGround;
#endif
vec4 shadowsInfo;
vec2 depthValues;
} light1;
#ifdef PROJECTEDLIGHTTEXTURE1
uniform mat4 textureProjectionMatrix1;
uniform sampler2D projectionLightSampler1;
#endif
#ifdef SHADOW1
#if defined(SHADOWCUBE1)
uniform samplerCube shadowSampler1; 
#else
out vec4 vPositionFromLight1;
out float vDepthMetric1;
#if defined(SHADOWPCSS1)
uniform highp sampler2DShadow shadowSampler1;
uniform highp sampler2D depthSampler1;
#elif defined(SHADOWPCF1)
uniform highp sampler2DShadow shadowSampler1;
#else
uniform sampler2D shadowSampler1;
#endif
uniform mat4 lightMatrix1;
#endif
#endif
#endif
#ifdef LIGHT2
uniform Light2
{
vec4 vLightData;
vec4 vLightDiffuse;
vec3 vLightSpecular;
#ifdef SPOTLIGHT2
vec4 vLightDirection;
vec4 vLightFalloff;
#elif defined(POINTLIGHT2)
vec4 vLightFalloff;
#elif defined(HEMILIGHT2)
vec3 vLightGround;
#endif
vec4 shadowsInfo;
vec2 depthValues;
} light2;
#ifdef PROJECTEDLIGHTTEXTURE2
uniform mat4 textureProjectionMatrix2;
uniform sampler2D projectionLightSampler2;
#endif
#ifdef SHADOW2
#if defined(SHADOWCUBE2)
uniform samplerCube shadowSampler2; 
#else
out vec4 vPositionFromLight2;
out float vDepthMetric2;
#if defined(SHADOWPCSS2)
uniform highp sampler2DShadow shadowSampler2;
uniform highp sampler2D depthSampler2;
#elif defined(SHADOWPCF2)
uniform highp sampler2DShadow shadowSampler2;
#else
uniform sampler2D shadowSampler2;
#endif
uniform mat4 lightMatrix2;
#endif
#endif
#endif
#ifdef LIGHT3
uniform Light3
{
vec4 vLightData;
vec4 vLightDiffuse;
vec3 vLightSpecular;
#ifdef SPOTLIGHT3
vec4 vLightDirection;
vec4 vLightFalloff;
#elif defined(POINTLIGHT3)
vec4 vLightFalloff;
#elif defined(HEMILIGHT3)
vec3 vLightGround;
#endif
vec4 shadowsInfo;
vec2 depthValues;
} light3;
#ifdef PROJECTEDLIGHTTEXTURE3
uniform mat4 textureProjectionMatrix3;
uniform sampler2D projectionLightSampler3;
#endif
#ifdef SHADOW3
#if defined(SHADOWCUBE3)
uniform samplerCube shadowSampler3; 
#else
out vec4 vPositionFromLight3;
out float vDepthMetric3;
#if defined(SHADOWPCSS3)
uniform highp sampler2DShadow shadowSampler3;
uniform highp sampler2D depthSampler3;
#elif defined(SHADOWPCF3)
uniform highp sampler2DShadow shadowSampler3;
#else
uniform sampler2D shadowSampler3;
#endif
uniform mat4 lightMatrix3;
#endif
#endif
#endif

#ifdef MORPHTARGETS
uniform float morphTargetInfluences[NUM_MORPH_INFLUENCERS];
#endif

#ifdef REFLECTIONMAP_SKYBOX
out vec3 vPositionUVW;
#endif
#if defined(REFLECTIONMAP_EQUIRECTANGULAR_FIXED) || defined(REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED)
out vec3 vDirectionW;
#endif
#ifdef LOGARITHMICDEPTH
uniform float logarithmicDepthConstant;
out float vFragmentDepth;
#endif
void main(void) {
vec3 positionUpdated=position;
#ifdef NORMAL
vec3 normalUpdated=normal;
#endif
#ifdef TANGENT
vec4 tangentUpdated=tangent;
#endif

#ifdef REFLECTIONMAP_SKYBOX
#ifdef REFLECTIONMAP_SKYBOX_TRANSFORMED
vPositionUVW=(reflectionMatrix*vec4(positionUpdated,1.0)).xyz;
#else
vPositionUVW=positionUpdated;
#endif
#endif 
#ifdef INSTANCES
mat4 finalWorld=mat4(world0,world1,world2,world3);
#else
mat4 finalWorld=world;
#endif
#if NUM_BONE_INFLUENCERS>0
mat4 influence;
influence=mBones[int(matricesIndices[0])]*matricesWeights[0];
#if NUM_BONE_INFLUENCERS>1
influence+=mBones[int(matricesIndices[1])]*matricesWeights[1];
#endif 
#if NUM_BONE_INFLUENCERS>2
influence+=mBones[int(matricesIndices[2])]*matricesWeights[2];
#endif 
#if NUM_BONE_INFLUENCERS>3
influence+=mBones[int(matricesIndices[3])]*matricesWeights[3];
#endif 
#if NUM_BONE_INFLUENCERS>4
influence+=mBones[int(matricesIndicesExtra[0])]*matricesWeightsExtra[0];
#endif 
#if NUM_BONE_INFLUENCERS>5
influence+=mBones[int(matricesIndicesExtra[1])]*matricesWeightsExtra[1];
#endif 
#if NUM_BONE_INFLUENCERS>6
influence+=mBones[int(matricesIndicesExtra[2])]*matricesWeightsExtra[2];
#endif 
#if NUM_BONE_INFLUENCERS>7
influence+=mBones[int(matricesIndicesExtra[3])]*matricesWeightsExtra[3];
#endif 
finalWorld=finalWorld*influence;
#endif
gl_Position=viewProjection*finalWorld*vec4(positionUpdated,1.0);
vec4 worldPos=finalWorld*vec4(positionUpdated,1.0);
vPositionW=vec3(worldPos);
#ifdef NORMAL
mat3 normalWorld=mat3(finalWorld);
#ifdef NONUNIFORMSCALING
normalWorld=transposeMat3(inverseMat3(normalWorld));
#endif
vNormalW=normalize(normalWorld*normalUpdated);
#if defined(USESPHERICALFROMREFLECTIONMAP) && defined(USESPHERICALINVERTEX)
vec3 reflectionVector=vec3(reflectionMatrix*vec4(vNormalW,0)).xyz;
#ifdef REFLECTIONMAP_OPPOSITEZ
reflectionVector.z*=-1.0;
#endif
vEnvironmentIrradiance=environmentIrradianceJones(reflectionVector);
#endif
#endif
#if defined(REFLECTIONMAP_EQUIRECTANGULAR_FIXED) || defined(REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED)
vDirectionW=normalize(vec3(finalWorld*vec4(positionUpdated,0.0)));
#endif

#ifndef UV1
vec2 uv=vec2(0.,0.);
#endif
#ifndef UV2
vec2 uv2=vec2(0.,0.);
#endif
#ifdef MAINUV1
vMainUV1=uv;
#endif 
#ifdef MAINUV2
vMainUV2=uv2;
#endif 
#if defined(ALBEDO) && ALBEDODIRECTUV == 0 
if (vAlbedoInfos.x == 0.)
{
vAlbedoUV=vec2(albedoMatrix*vec4(uv,1.0,0.0));
}
else
{
vAlbedoUV=vec2(albedoMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(AMBIENT) && AMBIENTDIRECTUV == 0 
if (vAmbientInfos.x == 0.)
{
vAmbientUV=vec2(ambientMatrix*vec4(uv,1.0,0.0));
}
else
{
vAmbientUV=vec2(ambientMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(OPACITY) && OPACITYDIRECTUV == 0 
if (vOpacityInfos.x == 0.)
{
vOpacityUV=vec2(opacityMatrix*vec4(uv,1.0,0.0));
}
else
{
vOpacityUV=vec2(opacityMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(EMISSIVE) && EMISSIVEDIRECTUV == 0 
if (vEmissiveInfos.x == 0.)
{
vEmissiveUV=vec2(emissiveMatrix*vec4(uv,1.0,0.0));
}
else
{
vEmissiveUV=vec2(emissiveMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(LIGHTMAP) && LIGHTMAPDIRECTUV == 0 
if (vLightmapInfos.x == 0.)
{
vLightmapUV=vec2(lightmapMatrix*vec4(uv,1.0,0.0));
}
else
{
vLightmapUV=vec2(lightmapMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(REFLECTIVITY) && REFLECTIVITYDIRECTUV == 0 
if (vReflectivityInfos.x == 0.)
{
vReflectivityUV=vec2(reflectivityMatrix*vec4(uv,1.0,0.0));
}
else
{
vReflectivityUV=vec2(reflectivityMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(MICROSURFACEMAP) && MICROSURFACEMAPDIRECTUV == 0 
if (vMicroSurfaceSamplerInfos.x == 0.)
{
vMicroSurfaceSamplerUV=vec2(microSurfaceSamplerMatrix*vec4(uv,1.0,0.0));
}
else
{
vMicroSurfaceSamplerUV=vec2(microSurfaceSamplerMatrix*vec4(uv2,1.0,0.0));
}
#endif
#if defined(BUMP) && BUMPDIRECTUV == 0 
if (vBumpInfos.x == 0.)
{
vBumpUV=vec2(bumpMatrix*vec4(uv,1.0,0.0));
}
else
{
vBumpUV=vec2(bumpMatrix*vec4(uv2,1.0,0.0));
}
#endif

#if defined(BUMP) || defined(PARALLAX)
#if defined(TANGENT) && defined(NORMAL)
vec3 tbnNormal=normalize(normalUpdated);
vec3 tbnTangent=normalize(tangentUpdated.xyz);
vec3 tbnBitangent=cross(tbnNormal,tbnTangent)*tangentUpdated.w;
vTBN=mat3(finalWorld)*mat3(tbnTangent,tbnBitangent,tbnNormal);
#endif
#endif

#ifdef CLIPPLANE
fClipDistance=dot(worldPos,vClipPlane);
#endif
#ifdef CLIPPLANE2
fClipDistance2=dot(worldPos,vClipPlane2);
#endif
#ifdef CLIPPLANE3
fClipDistance3=dot(worldPos,vClipPlane3);
#endif
#ifdef CLIPPLANE4
fClipDistance4=dot(worldPos,vClipPlane4);
#endif

#ifdef FOG
vFogDistance=(view*worldPos).xyz;
#endif

#ifdef SHADOWS
#if defined(SHADOW0) && !defined(SHADOWCUBE0)
vPositionFromLight0=lightMatrix0*worldPos;
vDepthMetric0=((vPositionFromLight0.z+light0.depthValues.x)/(light0.depthValues.y));
#endif
#endif
#ifdef SHADOWS
#if defined(SHADOW1) && !defined(SHADOWCUBE1)
vPositionFromLight1=lightMatrix1*worldPos;
vDepthMetric1=((vPositionFromLight1.z+light1.depthValues.x)/(light1.depthValues.y));
#endif
#endif
#ifdef SHADOWS
#if defined(SHADOW2) && !defined(SHADOWCUBE2)
vPositionFromLight2=lightMatrix2*worldPos;
vDepthMetric2=((vPositionFromLight2.z+light2.depthValues.x)/(light2.depthValues.y));
#endif
#endif
#ifdef SHADOWS
#if defined(SHADOW3) && !defined(SHADOWCUBE3)
vPositionFromLight3=lightMatrix3*worldPos;
vDepthMetric3=((vPositionFromLight3.z+light3.depthValues.x)/(light3.depthValues.y));
#endif
#endif


#ifdef VERTEXCOLOR
vColor=color;
#endif

#ifdef POINTSIZE
gl_PointSize=pointSize;
#endif

#ifdef LOGARITHMICDEPTH
vFragmentDepth=1.0+gl_Position.w;
gl_Position.z=log2(max(0.000001,vFragmentDepth))*logarithmicDepthConstant;
#endif
}