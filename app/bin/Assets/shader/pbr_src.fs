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

#define SHADER_NAME fragment:pbr
#if defined(BUMP) || !defined(NORMAL) || defined(FORCENORMALFORWARD) || defined(SPECULARAA)

#endif
#ifdef LODBASEDMICROSFURACE

#endif
#ifdef LOGARITHMICDEPTH

#endif
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
uniform vec4 vEyePosition;
uniform vec3 vAmbientColor;
uniform vec4 vCameraInfos;

in vec3 vPositionW;
#ifdef MAINUV1
in vec2 vMainUV1;
#endif 
#ifdef MAINUV2 
in vec2 vMainUV2;
#endif 
#ifdef NORMAL
in vec3 vNormalW;
#if defined(USESPHERICALFROMREFLECTIONMAP) && defined(USESPHERICALINVERTEX)
in vec3 vEnvironmentIrradiance;
#endif
#endif
#ifdef VERTEXCOLOR
in vec4 vColor;
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
in vec4 vPositionFromLight0;
in float vDepthMetric0;
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
in vec4 vPositionFromLight1;
in float vDepthMetric1;
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
in vec4 vPositionFromLight2;
in float vDepthMetric2;
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
in vec4 vPositionFromLight3;
in float vDepthMetric3;
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


#ifdef ALBEDO
#if ALBEDODIRECTUV == 1
#define vAlbedoUV vMainUV1
#elif ALBEDODIRECTUV == 2
#define vAlbedoUV vMainUV2
#else
in vec2 vAlbedoUV;
#endif
uniform sampler2D albedoSampler;
#endif
#ifdef AMBIENT
#if AMBIENTDIRECTUV == 1
#define vAmbientUV vMainUV1
#elif AMBIENTDIRECTUV == 2
#define vAmbientUV vMainUV2
#else
in vec2 vAmbientUV;
#endif
uniform sampler2D ambientSampler;
#endif
#ifdef OPACITY
#if OPACITYDIRECTUV == 1
#define vOpacityUV vMainUV1
#elif OPACITYDIRECTUV == 2
#define vOpacityUV vMainUV2
#else
in vec2 vOpacityUV;
#endif
uniform sampler2D opacitySampler;
#endif
#ifdef EMISSIVE
#if EMISSIVEDIRECTUV == 1
#define vEmissiveUV vMainUV1
#elif EMISSIVEDIRECTUV == 2
#define vEmissiveUV vMainUV2
#else
in vec2 vEmissiveUV;
#endif
uniform sampler2D emissiveSampler;
#endif
#ifdef LIGHTMAP
#if LIGHTMAPDIRECTUV == 1
#define vLightmapUV vMainUV1
#elif LIGHTMAPDIRECTUV == 2
#define vLightmapUV vMainUV2
#else
in vec2 vLightmapUV;
#endif
uniform sampler2D lightmapSampler;
#endif
#ifdef REFLECTIVITY
#if REFLECTIVITYDIRECTUV == 1
#define vReflectivityUV vMainUV1
#elif REFLECTIVITYDIRECTUV == 2
#define vReflectivityUV vMainUV2
#else
in vec2 vReflectivityUV;
#endif
uniform sampler2D reflectivitySampler;
#endif
#ifdef MICROSURFACEMAP
#if MICROSURFACEMAPDIRECTUV == 1
#define vMicroSurfaceSamplerUV vMainUV1
#elif MICROSURFACEMAPDIRECTUV == 2
#define vMicroSurfaceSamplerUV vMainUV2
#else
in vec2 vMicroSurfaceSamplerUV;
#endif
uniform sampler2D microSurfaceSampler;
#endif

#ifdef REFRACTION
#ifdef REFRACTIONMAP_3D
#define sampleRefraction(s,c) texture(s,c)
uniform samplerCube refractionSampler;
#ifdef LODBASEDMICROSFURACE
#define sampleRefractionLod(s,c,l) textureLod(s,c,l)
#else
uniform samplerCube refractionSamplerLow;
uniform samplerCube refractionSamplerHigh;
#endif
#else
#define sampleRefraction(s,c) texture(s,c)
uniform sampler2D refractionSampler;
#ifdef LODBASEDMICROSFURACE
#define sampleRefractionLod(s,c,l) textureLod(s,c,l)
#else
uniform samplerCube refractionSamplerLow;
uniform samplerCube refractionSamplerHigh;
#endif
#endif
#endif

#ifdef REFLECTION
#ifdef REFLECTIONMAP_3D
#define sampleReflection(s,c) texture(s,c)
uniform samplerCube reflectionSampler;
#ifdef LODBASEDMICROSFURACE
#define sampleReflectionLod(s,c,l) textureLod(s,c,l)
#else
uniform samplerCube reflectionSamplerLow;
uniform samplerCube reflectionSamplerHigh;
#endif
#else
#define sampleReflection(s,c) texture(s,c)
uniform sampler2D reflectionSampler;
#ifdef LODBASEDMICROSFURACE
#define sampleReflectionLod(s,c,l) textureLod(s,c,l)
#else
uniform samplerCube reflectionSamplerLow;
uniform samplerCube reflectionSamplerHigh;
#endif
#endif
#ifdef REFLECTIONMAP_SKYBOX
in vec3 vPositionUVW;
#else
#if defined(REFLECTIONMAP_EQUIRECTANGULAR_FIXED) || defined(REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED)
in vec3 vDirectionW;
#endif
#endif
#ifdef USE_LOCAL_REFLECTIONMAP_CUBIC
vec3 parallaxCorrectNormal( vec3 vertexPos,vec3 origVec,vec3 cubeSize,vec3 cubePos ) {

vec3 invOrigVec=vec3(1.0,1.0,1.0)/origVec;
vec3 halfSize=cubeSize*0.5;
vec3 intersecAtMaxPlane=(cubePos+halfSize-vertexPos)*invOrigVec;
vec3 intersecAtMinPlane=(cubePos-halfSize-vertexPos)*invOrigVec;

vec3 largestIntersec=max(intersecAtMaxPlane,intersecAtMinPlane);

float distance=min(min(largestIntersec.x,largestIntersec.y),largestIntersec.z);

vec3 intersectPositionWS=vertexPos+origVec*distance;

return intersectPositionWS-cubePos;
}
#endif
vec3 computeReflectionCoords(vec4 worldPos,vec3 worldNormal)
{
#if defined(REFLECTIONMAP_EQUIRECTANGULAR_FIXED) || defined(REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED)
vec3 direction=normalize(vDirectionW);
float lon=atan(direction.z,direction.x);
float lat=acos(direction.y);
vec2 sphereCoords=vec2(lon,lat)*RECIPROCAL_PI2*2.0;
float s=sphereCoords.x*0.5+0.5;
float t=sphereCoords.y; 
#ifdef REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED
return vec3(1.0-s,t,0);
#else
return vec3(s,t,0);
#endif
#endif
#ifdef REFLECTIONMAP_EQUIRECTANGULAR
vec3 cameraToVertex=normalize(worldPos.xyz-vEyePosition.xyz);
vec3 r=normalize(reflect(cameraToVertex,worldNormal));
float lon=atan(r.z,r.x);
float lat=acos(r.y);
vec2 sphereCoords=vec2(lon,lat)*RECIPROCAL_PI2*2.0;
float s=sphereCoords.x*0.5+0.5;
float t=sphereCoords.y; 
return vec3(s,t,0);
#endif
#ifdef REFLECTIONMAP_SPHERICAL
vec3 viewDir=normalize(vec3(view*worldPos));
vec3 viewNormal=normalize(vec3(view*vec4(worldNormal,0.0)));
vec3 r=reflect(viewDir,viewNormal);
r.z=r.z-1.0;
float m=2.0*length(r);
return vec3(r.x/m+0.5,1.0-r.y/m-0.5,0);
#endif
#ifdef REFLECTIONMAP_PLANAR
vec3 viewDir=worldPos.xyz-vEyePosition.xyz;
vec3 coords=normalize(reflect(viewDir,worldNormal));
return vec3(reflectionMatrix*vec4(coords,1));
#endif
#ifdef REFLECTIONMAP_CUBIC
vec3 viewDir=normalize(worldPos.xyz-vEyePosition.xyz);

vec3 coords=reflect(viewDir,worldNormal);
#ifdef USE_LOCAL_REFLECTIONMAP_CUBIC
coords=parallaxCorrectNormal(worldPos.xyz,coords,vReflectionSize,vReflectionPosition);
#endif
coords=vec3(reflectionMatrix*vec4(coords,0));
#ifdef INVERTCUBICMAP
coords.y*=-1.0;
#endif
return coords;
#endif
#ifdef REFLECTIONMAP_PROJECTION
return vec3(reflectionMatrix*(view*worldPos));
#endif
#ifdef REFLECTIONMAP_SKYBOX
return vPositionUVW;
#endif
#ifdef REFLECTIONMAP_EXPLICIT
return vec3(0,0,0);
#endif
}
#endif
#ifdef ENVIRONMENTBRDF
uniform sampler2D environmentBrdfSampler;
#endif

#ifndef FROMLINEARSPACE
#define FROMLINEARSPACE;
#endif
#ifdef EXPOSURE
uniform float exposureLinear;
#endif
#ifdef CONTRAST
uniform float contrast;
#endif
#ifdef VIGNETTE
uniform vec2 vInverseScreenSize;
uniform vec4 vignetteSettings1;
uniform vec4 vignetteSettings2;
#endif
#ifdef COLORCURVES
uniform vec4 vCameraColorCurveNegative;
uniform vec4 vCameraColorCurveNeutral;
uniform vec4 vCameraColorCurvePositive;
#endif
#ifdef COLORGRADING
#ifdef COLORGRADING3D
uniform highp sampler3D txColorTransform;
#else
uniform sampler2D txColorTransform;
#endif
uniform vec4 colorTransformSettings;
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
#if defined(COLORGRADING) && !defined(COLORGRADING3D)

vec3 sampleTexture3D(sampler2D colorTransform,vec3 color,vec2 sampler3dSetting)
{
float sliceSize=2.0*sampler3dSetting.x; 
#ifdef SAMPLER3DGREENDEPTH
float sliceContinuous=(color.g-sampler3dSetting.x)*sampler3dSetting.y;
#else
float sliceContinuous=(color.b-sampler3dSetting.x)*sampler3dSetting.y;
#endif
float sliceInteger=floor(sliceContinuous);


float sliceFraction=sliceContinuous-sliceInteger;
#ifdef SAMPLER3DGREENDEPTH
vec2 sliceUV=color.rb;
#else
vec2 sliceUV=color.rg;
#endif
sliceUV.x*=sliceSize;
sliceUV.x+=sliceInteger*sliceSize;
sliceUV=clamp(sliceUV,0.,1.);
vec4 slice0Color=texture(colorTransform,sliceUV);
sliceUV.x+=sliceSize;
sliceUV=clamp(sliceUV,0.,1.);
vec4 slice1Color=texture(colorTransform,sliceUV);
vec3 result=mix(slice0Color.rgb,slice1Color.rgb,sliceFraction);
#ifdef SAMPLER3DBGRMAP
color.rgb=result.rgb;
#else
color.rgb=result.bgr;
#endif
return color;
}
#endif
#ifdef TONEMAPPING_ACES





const mat3 ACESInputMat=mat3(
vec3(0.59719,0.07600,0.02840),
vec3(0.35458,0.90834,0.13383),
vec3(0.04823,0.01566,0.83777)
);

const mat3 ACESOutputMat=mat3(
vec3( 1.60475,-0.10208,-0.00327),
vec3(-0.53108,1.10813,-0.07276),
vec3(-0.07367,-0.00605,1.07602)
);
vec3 RRTAndODTFit(vec3 v)
{
vec3 a=v*(v+0.0245786)-0.000090537;
vec3 b=v*(0.983729*v+0.4329510)+0.238081;
return a/b;
}
vec3 ACESFitted(vec3 color)
{
color=ACESInputMat*color;

color=RRTAndODTFit(color);
color=ACESOutputMat*color;

color=clamp(color,0.0,1.0);
return color;
}
#endif
vec4 applyImageProcessing(vec4 result) {
#ifdef EXPOSURE
result.rgb*=exposureLinear;
#endif
#ifdef VIGNETTE

vec2 viewportXY=gl_FragCoord.xy*vInverseScreenSize;
viewportXY=viewportXY*2.0-1.0;
vec3 vignetteXY1=vec3(viewportXY*vignetteSettings1.xy+vignetteSettings1.zw,1.0);
float vignetteTerm=dot(vignetteXY1,vignetteXY1);
float vignette=pow(vignetteTerm,vignetteSettings2.w);

vec3 vignetteColor=vignetteSettings2.rgb;
#ifdef VIGNETTEBLENDMODEMULTIPLY
vec3 vignetteColorMultiplier=mix(vignetteColor,vec3(1,1,1),vignette);
result.rgb*=vignetteColorMultiplier;
#endif
#ifdef VIGNETTEBLENDMODEOPAQUE
result.rgb=mix(vignetteColor,result.rgb,vignette);
#endif
#endif
#ifdef TONEMAPPING
#ifdef TONEMAPPING_ACES
result.rgb=ACESFitted(result.rgb);
#else
const float tonemappingCalibration=1.590579;
result.rgb=1.0-exp2(-tonemappingCalibration*result.rgb);
#endif
#endif

result.rgb=toGammaSpace(result.rgb);
result.rgb=clamp(result.rgb,0.0,1.0);
#ifdef CONTRAST

vec3 resultHighContrast=applyEaseInOut(result.rgb);
if (contrast<1.0) {

result.rgb=mix(vec3(0.5,0.5,0.5),result.rgb,contrast);
} else {

result.rgb=mix(result.rgb,resultHighContrast,contrast-1.0);
}
#endif

#ifdef COLORGRADING
vec3 colorTransformInput=result.rgb*colorTransformSettings.xxx+colorTransformSettings.yyy;
#ifdef COLORGRADING3D
vec3 colorTransformOutput=texture(txColorTransform,colorTransformInput).rgb;
#else
vec3 colorTransformOutput=sampleTexture3D(txColorTransform,colorTransformInput,colorTransformSettings.yz).rgb;
#endif
result.rgb=mix(result.rgb,colorTransformOutput,colorTransformSettings.www);
#endif
#ifdef COLORCURVES

float luma=getLuminance(result.rgb);
vec2 curveMix=clamp(vec2(luma*3.0-1.5,luma*-3.0+1.5),vec2(0.0),vec2(1.0));
vec4 colorCurve=vCameraColorCurveNeutral+curveMix.x*vCameraColorCurvePositive-curveMix.y*vCameraColorCurveNegative;
result.rgb*=colorCurve.rgb;
result.rgb=mix(vec3(luma),result.rgb,colorCurve.a);
#endif
return result;
}

#ifdef SHADOWS
#ifndef SHADOWFLOAT
float unpack(vec4 color)
{
const vec4 bit_shift=vec4(1.0/(255.0*255.0*255.0),1.0/(255.0*255.0),1.0/255.0,1.0);
return dot(color,bit_shift);
}
#endif
float computeShadowCube(vec3 lightPosition,samplerCube shadowSampler,float darkness,vec2 depthValues)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=(depth+depthValues.x)/(depthValues.y);
depth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y=-directionToLight.y;
#ifndef SHADOWFLOAT
float shadow=unpack(texture(shadowSampler,directionToLight));
#else
float shadow=texture(shadowSampler,directionToLight).x;
#endif
if (depth>shadow)
{
return darkness;
}
return 1.0;
}
float computeShadowWithPoissonSamplingCube(vec3 lightPosition,samplerCube shadowSampler,float mapSize,float darkness,vec2 depthValues)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=(depth+depthValues.x)/(depthValues.y);
depth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y=-directionToLight.y;
float visibility=1.;
vec3 poissonDisk[4];
poissonDisk[0]=vec3(-1.0,1.0,-1.0);
poissonDisk[1]=vec3(1.0,-1.0,-1.0);
poissonDisk[2]=vec3(-1.0,-1.0,-1.0);
poissonDisk[3]=vec3(1.0,-1.0,1.0);

#ifndef SHADOWFLOAT
if (unpack(texture(shadowSampler,directionToLight+poissonDisk[0]*mapSize))<depth) visibility-=0.25;
if (unpack(texture(shadowSampler,directionToLight+poissonDisk[1]*mapSize))<depth) visibility-=0.25;
if (unpack(texture(shadowSampler,directionToLight+poissonDisk[2]*mapSize))<depth) visibility-=0.25;
if (unpack(texture(shadowSampler,directionToLight+poissonDisk[3]*mapSize))<depth) visibility-=0.25;
#else
if (texture(shadowSampler,directionToLight+poissonDisk[0]*mapSize).x<depth) visibility-=0.25;
if (texture(shadowSampler,directionToLight+poissonDisk[1]*mapSize).x<depth) visibility-=0.25;
if (texture(shadowSampler,directionToLight+poissonDisk[2]*mapSize).x<depth) visibility-=0.25;
if (texture(shadowSampler,directionToLight+poissonDisk[3]*mapSize).x<depth) visibility-=0.25;
#endif
return min(1.0,visibility+darkness);
}
float computeShadowWithESMCube(vec3 lightPosition,samplerCube shadowSampler,float darkness,float depthScale,vec2 depthValues)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=(depth+depthValues.x)/(depthValues.y);
float shadowPixelDepth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y=-directionToLight.y;
#ifndef SHADOWFLOAT
float shadowMapSample=unpack(texture(shadowSampler,directionToLight));
#else
float shadowMapSample=texture(shadowSampler,directionToLight).x;
#endif
float esm=1.0-clamp(exp(min(87.,depthScale*shadowPixelDepth))*shadowMapSample,0.,1.-darkness); 
return esm;
}
float computeShadowWithCloseESMCube(vec3 lightPosition,samplerCube shadowSampler,float darkness,float depthScale,vec2 depthValues)
{
vec3 directionToLight=vPositionW-lightPosition;
float depth=length(directionToLight);
depth=(depth+depthValues.x)/(depthValues.y);
float shadowPixelDepth=clamp(depth,0.,1.0);
directionToLight=normalize(directionToLight);
directionToLight.y=-directionToLight.y;
#ifndef SHADOWFLOAT
float shadowMapSample=unpack(texture(shadowSampler,directionToLight));
#else
float shadowMapSample=texture(shadowSampler,directionToLight).x;
#endif
float esm=clamp(exp(min(87.,-depthScale*(shadowPixelDepth-shadowMapSample))),darkness,1.);
return esm;
}
float computeShadow(vec4 vPositionFromLight,float depthMetric,sampler2D shadowSampler,float darkness,float frustumEdgeFalloff)
{
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec2 uv=0.5*clipSpace.xy+vec2(0.5);
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
float shadowPixelDepth=clamp(depthMetric,0.,1.0);
#ifndef SHADOWFLOAT
float shadow=unpack(texture(shadowSampler,uv));
#else
float shadow=texture(shadowSampler,uv).x;
#endif
if (shadowPixelDepth>shadow)
{
return computeFallOff(darkness,clipSpace.xy,frustumEdgeFalloff);
}
return 1.;
}
float computeShadowWithPoissonSampling(vec4 vPositionFromLight,float depthMetric,sampler2D shadowSampler,float mapSize,float darkness,float frustumEdgeFalloff)
{
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec2 uv=0.5*clipSpace.xy+vec2(0.5);
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
float shadowPixelDepth=clamp(depthMetric,0.,1.0);
float visibility=1.;
vec2 poissonDisk[4];
poissonDisk[0]=vec2(-0.94201624,-0.39906216);
poissonDisk[1]=vec2(0.94558609,-0.76890725);
poissonDisk[2]=vec2(-0.094184101,-0.92938870);
poissonDisk[3]=vec2(0.34495938,0.29387760);

#ifndef SHADOWFLOAT
if (unpack(texture(shadowSampler,uv+poissonDisk[0]*mapSize))<shadowPixelDepth) visibility-=0.25;
if (unpack(texture(shadowSampler,uv+poissonDisk[1]*mapSize))<shadowPixelDepth) visibility-=0.25;
if (unpack(texture(shadowSampler,uv+poissonDisk[2]*mapSize))<shadowPixelDepth) visibility-=0.25;
if (unpack(texture(shadowSampler,uv+poissonDisk[3]*mapSize))<shadowPixelDepth) visibility-=0.25;
#else
if (texture(shadowSampler,uv+poissonDisk[0]*mapSize).x<shadowPixelDepth) visibility-=0.25;
if (texture(shadowSampler,uv+poissonDisk[1]*mapSize).x<shadowPixelDepth) visibility-=0.25;
if (texture(shadowSampler,uv+poissonDisk[2]*mapSize).x<shadowPixelDepth) visibility-=0.25;
if (texture(shadowSampler,uv+poissonDisk[3]*mapSize).x<shadowPixelDepth) visibility-=0.25;
#endif
return computeFallOff(min(1.0,visibility+darkness),clipSpace.xy,frustumEdgeFalloff);
}
float computeShadowWithESM(vec4 vPositionFromLight,float depthMetric,sampler2D shadowSampler,float darkness,float depthScale,float frustumEdgeFalloff)
{
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec2 uv=0.5*clipSpace.xy+vec2(0.5);
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
float shadowPixelDepth=clamp(depthMetric,0.,1.0);
#ifndef SHADOWFLOAT
float shadowMapSample=unpack(texture(shadowSampler,uv));
#else
float shadowMapSample=texture(shadowSampler,uv).x;
#endif
float esm=1.0-clamp(exp(min(87.,depthScale*shadowPixelDepth))*shadowMapSample,0.,1.-darkness);
return computeFallOff(esm,clipSpace.xy,frustumEdgeFalloff);
}
float computeShadowWithCloseESM(vec4 vPositionFromLight,float depthMetric,sampler2D shadowSampler,float darkness,float depthScale,float frustumEdgeFalloff)
{
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec2 uv=0.5*clipSpace.xy+vec2(0.5);
if (uv.x<0. || uv.x>1.0 || uv.y<0. || uv.y>1.0)
{
return 1.0;
}
float shadowPixelDepth=clamp(depthMetric,0.,1.0); 
#ifndef SHADOWFLOAT
float shadowMapSample=unpack(texture(shadowSampler,uv));
#else
float shadowMapSample=texture(shadowSampler,uv).x;
#endif
float esm=clamp(exp(min(87.,-depthScale*(shadowPixelDepth-shadowMapSample))),darkness,1.);
return computeFallOff(esm,clipSpace.xy,frustumEdgeFalloff);
}
#ifdef WEBGL2

float computeShadowWithPCF1(vec4 vPositionFromLight,float depthMetric,sampler2DShadow shadowSampler,float darkness,float frustumEdgeFalloff)
{
if (depthMetric>1.0 || depthMetric<0.0) {
return 1.0;
}
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec3 uvDepth=vec3(0.5*clipSpace.xyz+vec3(0.5));
float shadow=texture(shadowSampler,uvDepth);
shadow=mix(darkness,1.,shadow);
return computeFallOff(shadow,clipSpace.xy,frustumEdgeFalloff);
}



float computeShadowWithPCF3(vec4 vPositionFromLight,float depthMetric,sampler2DShadow shadowSampler,vec2 shadowMapSizeAndInverse,float darkness,float frustumEdgeFalloff)
{
if (depthMetric>1.0 || depthMetric<0.0) {
return 1.0;
}
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec3 uvDepth=vec3(0.5*clipSpace.xyz+vec3(0.5));
vec2 uv=uvDepth.xy*shadowMapSizeAndInverse.x; 
uv+=0.5; 
vec2 st=fract(uv); 
vec2 base_uv=floor(uv)-0.5; 
base_uv*=shadowMapSizeAndInverse.y; 




vec2 uvw0=3.-2.*st;
vec2 uvw1=1.+2.*st;
vec2 u=vec2((2.-st.x)/uvw0.x-1.,st.x/uvw1.x+1.)*shadowMapSizeAndInverse.y;
vec2 v=vec2((2.-st.y)/uvw0.y-1.,st.y/uvw1.y+1.)*shadowMapSizeAndInverse.y;
float shadow=0.;
shadow+=uvw0.x*uvw0.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[0],v[0]),uvDepth.z));
shadow+=uvw1.x*uvw0.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[1],v[0]),uvDepth.z));
shadow+=uvw0.x*uvw1.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[0],v[1]),uvDepth.z));
shadow+=uvw1.x*uvw1.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[1],v[1]),uvDepth.z));
shadow=shadow/16.;
shadow=mix(darkness,1.,shadow);
return computeFallOff(shadow,clipSpace.xy,frustumEdgeFalloff);
}



float computeShadowWithPCF5(vec4 vPositionFromLight,float depthMetric,sampler2DShadow shadowSampler,vec2 shadowMapSizeAndInverse,float darkness,float frustumEdgeFalloff)
{
if (depthMetric>1.0 || depthMetric<0.0) {
return 1.0;
}
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec3 uvDepth=vec3(0.5*clipSpace.xyz+vec3(0.5));
vec2 uv=uvDepth.xy*shadowMapSizeAndInverse.x; 
uv+=0.5; 
vec2 st=fract(uv); 
vec2 base_uv=floor(uv)-0.5; 
base_uv*=shadowMapSizeAndInverse.y; 


vec2 uvw0=4.-3.*st;
vec2 uvw1=vec2(7.);
vec2 uvw2=1.+3.*st;
vec3 u=vec3((3.-2.*st.x)/uvw0.x-2.,(3.+st.x)/uvw1.x,st.x/uvw2.x+2.)*shadowMapSizeAndInverse.y;
vec3 v=vec3((3.-2.*st.y)/uvw0.y-2.,(3.+st.y)/uvw1.y,st.y/uvw2.y+2.)*shadowMapSizeAndInverse.y;
float shadow=0.;
shadow+=uvw0.x*uvw0.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[0],v[0]),uvDepth.z));
shadow+=uvw1.x*uvw0.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[1],v[0]),uvDepth.z));
shadow+=uvw2.x*uvw0.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[2],v[0]),uvDepth.z));
shadow+=uvw0.x*uvw1.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[0],v[1]),uvDepth.z));
shadow+=uvw1.x*uvw1.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[1],v[1]),uvDepth.z));
shadow+=uvw2.x*uvw1.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[2],v[1]),uvDepth.z));
shadow+=uvw0.x*uvw2.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[0],v[2]),uvDepth.z));
shadow+=uvw1.x*uvw2.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[1],v[2]),uvDepth.z));
shadow+=uvw2.x*uvw2.y*texture(shadowSampler,vec3(base_uv.xy+vec2(u[2],v[2]),uvDepth.z));
shadow=shadow/144.;
shadow=mix(darkness,1.,shadow);
return computeFallOff(shadow,clipSpace.xy,frustumEdgeFalloff);
}
const vec3 PoissonSamplers32[64]=vec3[64](
vec3(0.06407013,0.05409927,0.),
vec3(0.7366577,0.5789394,0.),
vec3(-0.6270542,-0.5320278,0.),
vec3(-0.4096107,0.8411095,0.),
vec3(0.6849564,-0.4990818,0.),
vec3(-0.874181,-0.04579735,0.),
vec3(0.9989998,0.0009880066,0.),
vec3(-0.004920578,-0.9151649,0.),
vec3(0.1805763,0.9747483,0.),
vec3(-0.2138451,0.2635818,0.),
vec3(0.109845,0.3884785,0.),
vec3(0.06876755,-0.3581074,0.),
vec3(0.374073,-0.7661266,0.),
vec3(0.3079132,-0.1216763,0.),
vec3(-0.3794335,-0.8271583,0.),
vec3(-0.203878,-0.07715034,0.),
vec3(0.5912697,0.1469799,0.),
vec3(-0.88069,0.3031784,0.),
vec3(0.5040108,0.8283722,0.),
vec3(-0.5844124,0.5494877,0.),
vec3(0.6017799,-0.1726654,0.),
vec3(-0.5554981,0.1559997,0.),
vec3(-0.3016369,-0.3900928,0.),
vec3(-0.5550632,-0.1723762,0.),
vec3(0.925029,0.2995041,0.),
vec3(-0.2473137,0.5538505,0.),
vec3(0.9183037,-0.2862392,0.),
vec3(0.2469421,0.6718712,0.),
vec3(0.3916397,-0.4328209,0.),
vec3(-0.03576927,-0.6220032,0.),
vec3(-0.04661255,0.7995201,0.),
vec3(0.4402924,0.3640312,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.),
vec3(0.,0.,0.)
);
const vec3 PoissonSamplers64[64]=vec3[64](
vec3(-0.613392,0.617481,0.),
vec3(0.170019,-0.040254,0.),
vec3(-0.299417,0.791925,0.),
vec3(0.645680,0.493210,0.),
vec3(-0.651784,0.717887,0.),
vec3(0.421003,0.027070,0.),
vec3(-0.817194,-0.271096,0.),
vec3(-0.705374,-0.668203,0.),
vec3(0.977050,-0.108615,0.),
vec3(0.063326,0.142369,0.),
vec3(0.203528,0.214331,0.),
vec3(-0.667531,0.326090,0.),
vec3(-0.098422,-0.295755,0.),
vec3(-0.885922,0.215369,0.),
vec3(0.566637,0.605213,0.),
vec3(0.039766,-0.396100,0.),
vec3(0.751946,0.453352,0.),
vec3(0.078707,-0.715323,0.),
vec3(-0.075838,-0.529344,0.),
vec3(0.724479,-0.580798,0.),
vec3(0.222999,-0.215125,0.),
vec3(-0.467574,-0.405438,0.),
vec3(-0.248268,-0.814753,0.),
vec3(0.354411,-0.887570,0.),
vec3(0.175817,0.382366,0.),
vec3(0.487472,-0.063082,0.),
vec3(-0.084078,0.898312,0.),
vec3(0.488876,-0.783441,0.),
vec3(0.470016,0.217933,0.),
vec3(-0.696890,-0.549791,0.),
vec3(-0.149693,0.605762,0.),
vec3(0.034211,0.979980,0.),
vec3(0.503098,-0.308878,0.),
vec3(-0.016205,-0.872921,0.),
vec3(0.385784,-0.393902,0.),
vec3(-0.146886,-0.859249,0.),
vec3(0.643361,0.164098,0.),
vec3(0.634388,-0.049471,0.),
vec3(-0.688894,0.007843,0.),
vec3(0.464034,-0.188818,0.),
vec3(-0.440840,0.137486,0.),
vec3(0.364483,0.511704,0.),
vec3(0.034028,0.325968,0.),
vec3(0.099094,-0.308023,0.),
vec3(0.693960,-0.366253,0.),
vec3(0.678884,-0.204688,0.),
vec3(0.001801,0.780328,0.),
vec3(0.145177,-0.898984,0.),
vec3(0.062655,-0.611866,0.),
vec3(0.315226,-0.604297,0.),
vec3(-0.780145,0.486251,0.),
vec3(-0.371868,0.882138,0.),
vec3(0.200476,0.494430,0.),
vec3(-0.494552,-0.711051,0.),
vec3(0.612476,0.705252,0.),
vec3(-0.578845,-0.768792,0.),
vec3(-0.772454,-0.090976,0.),
vec3(0.504440,0.372295,0.),
vec3(0.155736,0.065157,0.),
vec3(0.391522,0.849605,0.),
vec3(-0.620106,-0.328104,0.),
vec3(0.789239,-0.419965,0.),
vec3(-0.545396,0.538133,0.),
vec3(-0.178564,-0.596057,0.)
);





float computeShadowWithPCSS(vec4 vPositionFromLight,float depthMetric,sampler2D depthSampler,sampler2DShadow shadowSampler,float shadowMapSizeInverse,float lightSizeUV,float darkness,float frustumEdgeFalloff,int searchTapCount,int pcfTapCount,vec3[64] poissonSamplers)
{
if (depthMetric>1.0 || depthMetric<0.0) {
return 1.0;
}
vec3 clipSpace=vPositionFromLight.xyz/vPositionFromLight.w;
vec3 uvDepth=vec3(0.5*clipSpace.xyz+vec3(0.5));
float blockerDepth=0.0;
float sumBlockerDepth=0.0;
float numBlocker=0.0;
for (int i=0; i<searchTapCount; i ++) {
blockerDepth=texture(depthSampler,uvDepth.xy+(lightSizeUV*shadowMapSizeInverse*PoissonSamplers32[i].xy)).r;
if (blockerDepth<depthMetric) {
sumBlockerDepth+=blockerDepth;
numBlocker++;
}
}
if (numBlocker<1.0) {
return 1.0;
}
float avgBlockerDepth=sumBlockerDepth/numBlocker;

float AAOffset=shadowMapSizeInverse*10.;


float penumbraRatio=((depthMetric-avgBlockerDepth)+AAOffset);
float filterRadius=penumbraRatio*lightSizeUV*shadowMapSizeInverse;
float random=getRand(vPositionFromLight.xy);
float rotationAngle=random*3.1415926;
vec2 rotationVector=vec2(cos(rotationAngle),sin(rotationAngle));
float shadow=0.;
for (int i=0; i<pcfTapCount; i++) {
vec3 offset=poissonSamplers[i];

offset=vec3(offset.x*rotationVector.x-offset.y*rotationVector.y,offset.y*rotationVector.x+offset.x*rotationVector.y,0.);
shadow+=texture(shadowSampler,uvDepth+offset*filterRadius);
}
shadow/=float(pcfTapCount);

shadow=mix(shadow,1.,depthMetric-avgBlockerDepth);

shadow=mix(darkness,1.,shadow);

return computeFallOff(shadow,clipSpace.xy,frustumEdgeFalloff);
}
float computeShadowWithPCSS16(vec4 vPositionFromLight,float depthMetric,sampler2D depthSampler,sampler2DShadow shadowSampler,float shadowMapSizeInverse,float lightSizeUV,float darkness,float frustumEdgeFalloff)
{
return computeShadowWithPCSS(vPositionFromLight,depthMetric,depthSampler,shadowSampler,shadowMapSizeInverse,lightSizeUV,darkness,frustumEdgeFalloff,16,16,PoissonSamplers32);
}
float computeShadowWithPCSS32(vec4 vPositionFromLight,float depthMetric,sampler2D depthSampler,sampler2DShadow shadowSampler,float shadowMapSizeInverse,float lightSizeUV,float darkness,float frustumEdgeFalloff)
{
return computeShadowWithPCSS(vPositionFromLight,depthMetric,depthSampler,shadowSampler,shadowMapSizeInverse,lightSizeUV,darkness,frustumEdgeFalloff,16,32,PoissonSamplers32);
}
float computeShadowWithPCSS64(vec4 vPositionFromLight,float depthMetric,sampler2D depthSampler,sampler2DShadow shadowSampler,float shadowMapSizeInverse,float lightSizeUV,float darkness,float frustumEdgeFalloff)
{
return computeShadowWithPCSS(vPositionFromLight,depthMetric,depthSampler,shadowSampler,shadowMapSizeInverse,lightSizeUV,darkness,frustumEdgeFalloff,32,64,PoissonSamplers64);
}
#endif
#endif


#define RECIPROCAL_PI2 0.15915494
#define FRESNEL_MAXIMUM_ON_ROUGH 0.25

const float kRougnhessToAlphaScale=0.1;
const float kRougnhessToAlphaOffset=0.29248125;
float convertRoughnessToAverageSlope(float roughness)
{

const float kMinimumVariance=0.0005;
float alphaG=square(roughness)+kMinimumVariance;
return alphaG;
}

float smithVisibilityG1_TrowbridgeReitzGGX(float dot,float alphaG)
{
float tanSquared=(1.0-dot*dot)/(dot*dot);
return 2.0/(1.0+sqrt(1.0+alphaG*alphaG*tanSquared));
}
float smithVisibilityG_TrowbridgeReitzGGX_Walter(float NdotL,float NdotV,float alphaG)
{
return smithVisibilityG1_TrowbridgeReitzGGX(NdotL,alphaG)*smithVisibilityG1_TrowbridgeReitzGGX(NdotV,alphaG);
}


float normalDistributionFunction_TrowbridgeReitzGGX(float NdotH,float alphaG)
{



float a2=square(alphaG);
float d=NdotH*NdotH*(a2-1.0)+1.0;
return a2/(PI*d*d);
}
vec3 fresnelSchlickGGX(float VdotH,vec3 reflectance0,vec3 reflectance90)
{
return reflectance0+(reflectance90-reflectance0)*pow(clamp(1.0-VdotH,0.,1.),5.0);
}
vec3 fresnelSchlickEnvironmentGGX(float VdotN,vec3 reflectance0,vec3 reflectance90,float smoothness)
{

float weight=mix(FRESNEL_MAXIMUM_ON_ROUGH,1.0,smoothness);
return reflectance0+weight*(reflectance90-reflectance0)*pow(clamp(1.0-VdotN,0.,1.),5.0);
}

vec3 computeSpecularTerm(float NdotH,float NdotL,float NdotV,float VdotH,float roughness,vec3 reflectance0,vec3 reflectance90,float geometricRoughnessFactor)
{
roughness=max(roughness,geometricRoughnessFactor);
float alphaG=convertRoughnessToAverageSlope(roughness);
float distribution=normalDistributionFunction_TrowbridgeReitzGGX(NdotH,alphaG);
float visibility=smithVisibilityG_TrowbridgeReitzGGX_Walter(NdotL,NdotV,alphaG);
visibility/=(4.0*NdotL*NdotV); 
float specTerm=max(0.,visibility*distribution)*NdotL;
vec3 fresnel=fresnelSchlickGGX(VdotH,reflectance0,reflectance90);
return fresnel*specTerm;
}
float computeDiffuseTerm(float NdotL,float NdotV,float VdotH,float roughness)
{


float diffuseFresnelNV=pow(clamp(1.0-NdotL,0.000001,1.),5.0);
float diffuseFresnelNL=pow(clamp(1.0-NdotV,0.000001,1.),5.0);
float diffuseFresnel90=0.5+2.0*VdotH*VdotH*roughness;
float fresnel =
(1.0+(diffuseFresnel90-1.0)*diffuseFresnelNL) *
(1.0+(diffuseFresnel90-1.0)*diffuseFresnelNV);
return fresnel*NdotL/PI;
}
float adjustRoughnessFromLightProperties(float roughness,float lightRadius,float lightDistance)
{
#if defined(USEPHYSICALLIGHTFALLOFF) || defined(USEGLTFLIGHTFALLOFF)

float lightRoughness=lightRadius/lightDistance;

float totalRoughness=clamp(lightRoughness+roughness,0.,1.);
return totalRoughness;
#else
return roughness;
#endif
}
float computeDefaultMicroSurface(float microSurface,vec3 reflectivityColor)
{
const float kReflectivityNoAlphaWorkflow_SmoothnessMax=0.95;
float reflectivityLuminance=getLuminance(reflectivityColor);
float reflectivityLuma=sqrt(reflectivityLuminance);
microSurface=reflectivityLuma*kReflectivityNoAlphaWorkflow_SmoothnessMax;
return microSurface;
}


float fresnelGrazingReflectance(float reflectance0) {
float reflectance90=clamp(reflectance0*25.0,0.0,1.0);
return reflectance90;
}


#define UNPACK_LOD(x) (1.0-x)*255.0
float getLodFromAlphaG(float cubeMapDimensionPixels,float alphaG,float NdotV) {
float microsurfaceAverageSlope=alphaG;






microsurfaceAverageSlope*=sqrt(abs(NdotV));
float microsurfaceAverageSlopeTexels=microsurfaceAverageSlope*cubeMapDimensionPixels;
float lod=log2(microsurfaceAverageSlopeTexels);
return lod;
}
float environmentRadianceOcclusion(float ambientOcclusion,float NdotVUnclamped) {


float temp=NdotVUnclamped+ambientOcclusion;
return clamp(square(temp)-1.0+ambientOcclusion,0.0,1.0);
}
float environmentHorizonOcclusion(vec3 view,vec3 normal) {

vec3 reflection=reflect(view,normal);
float temp=clamp( 1.0+1.1*dot(reflection,normal),0.0,1.0);
return square(temp);
}
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

struct lightingInfo
{
vec3 diffuse;
#ifdef SPECULARTERM
vec3 specular;
#endif
};
struct pointLightingInfo
{
vec3 lightOffset;
float lightDistanceSquared;
float attenuation;
};
struct spotLightingInfo
{
vec3 lightOffset;
float lightDistanceSquared;
vec3 directionToLightCenterW;
float attenuation;
};
float computeDistanceLightFalloff_Standard(vec3 lightOffset,float range)
{
return max(0.,1.0-length(lightOffset)/range);
}
float computeDistanceLightFalloff_Physical(float lightDistanceSquared)
{
return 1.0/((lightDistanceSquared+0.001));
}
float computeDistanceLightFalloff_GLTF(float lightDistanceSquared,float inverseSquaredRange)
{
const float minDistanceSquared=0.01*0.01;
float lightDistanceFalloff=1.0/(max(lightDistanceSquared,minDistanceSquared));
float factor=lightDistanceSquared*inverseSquaredRange;
float attenuation=clamp(1.0-factor*factor,0.,1.);
attenuation*=attenuation;

lightDistanceFalloff*=attenuation;
return lightDistanceFalloff;
}
float computeDistanceLightFalloff(vec3 lightOffset,float lightDistanceSquared,float range,float inverseSquaredRange)
{ 
#ifdef USEPHYSICALLIGHTFALLOFF
return computeDistanceLightFalloff_Physical(lightDistanceSquared);
#elif defined(USEGLTFLIGHTFALLOFF)
return computeDistanceLightFalloff_GLTF(lightDistanceSquared,inverseSquaredRange);
#else
return computeDistanceLightFalloff_Standard(lightOffset,range);
#endif
}
float computeDirectionalLightFalloff_Standard(vec3 lightDirection,vec3 directionToLightCenterW,float cosHalfAngle,float exponent)
{
float falloff=0.0;
float cosAngle=max(0.000000000000001,dot(-lightDirection,directionToLightCenterW));
if (cosAngle>=cosHalfAngle)
{
falloff=max(0.,pow(cosAngle,exponent));
}
return falloff;
}
float computeDirectionalLightFalloff_Physical(vec3 lightDirection,vec3 directionToLightCenterW,float cosHalfAngle)
{
const float kMinusLog2ConeAngleIntensityRatio=6.64385618977; 





float concentrationKappa=kMinusLog2ConeAngleIntensityRatio/(1.0-cosHalfAngle);


vec4 lightDirectionSpreadSG=vec4(-lightDirection*concentrationKappa,-concentrationKappa);
float falloff=exp2(dot(vec4(directionToLightCenterW,1.0),lightDirectionSpreadSG));
return falloff;
}
float computeDirectionalLightFalloff_GLTF(vec3 lightDirection,vec3 directionToLightCenterW,float lightAngleScale,float lightAngleOffset)
{



float cd=dot(-lightDirection,directionToLightCenterW);
float falloff=clamp(cd*lightAngleScale+lightAngleOffset,0.,1.);

falloff*=falloff;
return falloff;
}
float computeDirectionalLightFalloff(vec3 lightDirection,vec3 directionToLightCenterW,float cosHalfAngle,float exponent,float lightAngleScale,float lightAngleOffset)
{
#ifdef USEPHYSICALLIGHTFALLOFF
return computeDirectionalLightFalloff_Physical(lightDirection,directionToLightCenterW,cosHalfAngle);
#elif defined(USEGLTFLIGHTFALLOFF)
return computeDirectionalLightFalloff_GLTF(lightDirection,directionToLightCenterW,lightAngleScale,lightAngleOffset);
#else
return computeDirectionalLightFalloff_Standard(lightDirection,directionToLightCenterW,cosHalfAngle,exponent);
#endif
}
pointLightingInfo computePointLightingInfo(vec4 lightData) {
pointLightingInfo result;
result.lightOffset=lightData.xyz-vPositionW;
result.lightDistanceSquared=dot(result.lightOffset,result.lightOffset);
return result;
}
spotLightingInfo computeSpotLightingInfo(vec4 lightData) {
spotLightingInfo result;
result.lightOffset=lightData.xyz-vPositionW;
result.directionToLightCenterW=normalize(result.lightOffset);
result.lightDistanceSquared=dot(result.lightOffset,result.lightOffset);
return result;
}
lightingInfo computePointLighting(pointLightingInfo info,vec3 viewDirectionW,vec3 vNormal,vec3 diffuseColor,float lightRadius,float roughness,float NdotV,vec3 reflectance0,vec3 reflectance90,float geometricRoughnessFactor,out float NdotL) {
lightingInfo result;
float lightDistance=sqrt(info.lightDistanceSquared);
vec3 lightDirection=normalize(info.lightOffset);

roughness=adjustRoughnessFromLightProperties(roughness,lightRadius,lightDistance);

vec3 H=normalize(viewDirectionW+lightDirection);
NdotL=clamp(dot(vNormal,lightDirection),0.00000000001,1.0);
float VdotH=clamp(dot(viewDirectionW,H),0.0,1.0);
float diffuseTerm=computeDiffuseTerm(NdotL,NdotV,VdotH,roughness);
result.diffuse=diffuseTerm*diffuseColor*info.attenuation;
#ifdef SPECULARTERM

float NdotH=clamp(dot(vNormal,H),0.000000000001,1.0);
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,reflectance0,reflectance90,geometricRoughnessFactor);
result.specular=specTerm*diffuseColor*info.attenuation;
#endif
return result;
}
lightingInfo computeSpotLighting(spotLightingInfo info,vec3 viewDirectionW,vec3 vNormal,vec4 lightDirection,vec3 diffuseColor,float lightRadius,float roughness,float NdotV,vec3 reflectance0,vec3 reflectance90,float geometricRoughnessFactor,out float NdotL) {
lightingInfo result;

float lightDistance=sqrt(info.lightDistanceSquared);
roughness=adjustRoughnessFromLightProperties(roughness,lightRadius,lightDistance);

vec3 H=normalize(viewDirectionW+info.directionToLightCenterW);
NdotL=clamp(dot(vNormal,info.directionToLightCenterW),0.000000000001,1.0);
float VdotH=clamp(dot(viewDirectionW,H),0.0,1.0);
float diffuseTerm=computeDiffuseTerm(NdotL,NdotV,VdotH,roughness);
result.diffuse=diffuseTerm*diffuseColor*info.attenuation;
#ifdef SPECULARTERM

float NdotH=clamp(dot(vNormal,H),0.000000000001,1.0);
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,reflectance0,reflectance90,geometricRoughnessFactor);
result.specular=specTerm*diffuseColor*info.attenuation;
#endif
return result;
}
lightingInfo computeDirectionalLighting(vec3 viewDirectionW,vec3 vNormal,vec4 lightData,vec3 diffuseColor,vec3 specularColor,float lightRadius,float roughness,float NdotV,vec3 reflectance0,vec3 reflectance90,float geometricRoughnessFactor,out float NdotL) {
lightingInfo result;
float lightDistance=length(-lightData.xyz);
vec3 lightDirection=normalize(-lightData.xyz);

roughness=adjustRoughnessFromLightProperties(roughness,lightRadius,lightDistance);

vec3 H=normalize(viewDirectionW+lightDirection);
NdotL=clamp(dot(vNormal,lightDirection),0.00000000001,1.0);
float VdotH=clamp(dot(viewDirectionW,H),0.0,1.0);
float diffuseTerm=computeDiffuseTerm(NdotL,NdotV,VdotH,roughness);
result.diffuse=diffuseTerm*diffuseColor;
#ifdef SPECULARTERM

float NdotH=clamp(dot(vNormal,H),0.000000000001,1.0);
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,reflectance0,reflectance90,geometricRoughnessFactor);
result.specular=specTerm*diffuseColor;
#endif
return result;
}
lightingInfo computeHemisphericLighting(vec3 viewDirectionW,vec3 vNormal,vec4 lightData,vec3 diffuseColor,vec3 specularColor,vec3 groundColor,float roughness,float NdotV,vec3 reflectance0,vec3 reflectance90,float geometricRoughnessFactor,out float NdotL) {
lightingInfo result;



NdotL=dot(vNormal,lightData.xyz)*0.5+0.5;
result.diffuse=mix(groundColor,diffuseColor,NdotL);
#ifdef SPECULARTERM

vec3 lightVectorW=normalize(lightData.xyz);
vec3 H=normalize(viewDirectionW+lightVectorW);
float NdotH=clamp(dot(vNormal,H),0.000000000001,1.0);
NdotL=clamp(NdotL,0.000000000001,1.0);
float VdotH=clamp(dot(viewDirectionW,H),0.0,1.0);
vec3 specTerm=computeSpecularTerm(NdotH,NdotL,NdotV,VdotH,roughness,reflectance0,reflectance90,geometricRoughnessFactor);
result.specular=specTerm*diffuseColor;
#endif
return result;
}
vec3 computeProjectionTextureDiffuseLighting(sampler2D projectionLightSampler,mat4 textureProjectionMatrix){
vec4 strq=textureProjectionMatrix*vec4(vPositionW,1.0);
strq/=strq.w;
vec3 textureColor=texture(projectionLightSampler,strq.xy).rgb;
return toLinearSpace(textureColor);
}
#ifdef BUMP
#if BUMPDIRECTUV == 1
#define vBumpUV vMainUV1
#elif BUMPDIRECTUV == 2
#define vBumpUV vMainUV2
#else
in vec2 vBumpUV;
#endif
uniform sampler2D bumpSampler;
#if defined(TANGENT) && defined(NORMAL) 
in mat3 vTBN;
#endif
#ifdef OBJECTSPACE_NORMALMAP
uniform mat4 normalMatrix;
#endif

mat3 cotangent_frame(vec3 normal,vec3 p,vec2 uv)
{

uv=gl_FrontFacing ? uv : -uv;

vec3 dp1=dFdx(p);
vec3 dp2=dFdy(p);
vec2 duv1=dFdx(uv);
vec2 duv2=dFdy(uv);

vec3 dp2perp=cross(dp2,normal);
vec3 dp1perp=cross(normal,dp1);
vec3 tangent=dp2perp*duv1.x+dp1perp*duv2.x;
vec3 bitangent=dp2perp*duv1.y+dp1perp*duv2.y;

tangent*=vTangentSpaceParams.x;
bitangent*=vTangentSpaceParams.y;

float invmax=inversesqrt(max(dot(tangent,tangent),dot(bitangent,bitangent)));
return mat3(tangent*invmax,bitangent*invmax,normal);
}
vec3 perturbNormal(mat3 cotangentFrame,vec2 uv)
{
vec3 map=texture(bumpSampler,uv).xyz;
map=map*2.0-1.0;
#ifdef NORMALXYSCALE
map=normalize(map*vec3(vBumpInfos.y,vBumpInfos.y,1.0));
#endif
return normalize(cotangentFrame*map);
}
#ifdef PARALLAX
const float minSamples=4.;
const float maxSamples=15.;
const int iMaxSamples=15;

vec2 parallaxOcclusion(vec3 vViewDirCoT,vec3 vNormalCoT,vec2 texCoord,float parallaxScale) {
float parallaxLimit=length(vViewDirCoT.xy)/vViewDirCoT.z;
parallaxLimit*=parallaxScale;
vec2 vOffsetDir=normalize(vViewDirCoT.xy);
vec2 vMaxOffset=vOffsetDir*parallaxLimit;
float numSamples=maxSamples+(dot(vViewDirCoT,vNormalCoT)*(minSamples-maxSamples));
float stepSize=1.0/numSamples;

float currRayHeight=1.0;
vec2 vCurrOffset=vec2(0,0);
vec2 vLastOffset=vec2(0,0);
float lastSampledHeight=1.0;
float currSampledHeight=1.0;
for (int i=0; i<iMaxSamples; i++)
{
currSampledHeight=texture(bumpSampler,vBumpUV+vCurrOffset).w;

if (currSampledHeight>currRayHeight)
{
float delta1=currSampledHeight-currRayHeight;
float delta2=(currRayHeight+stepSize)-lastSampledHeight;
float ratio=delta1/(delta1+delta2);
vCurrOffset=(ratio)* vLastOffset+(1.0-ratio)*vCurrOffset;

break;
}
else
{
currRayHeight-=stepSize;
vLastOffset=vCurrOffset;
vCurrOffset+=stepSize*vMaxOffset;
lastSampledHeight=currSampledHeight;
}
}
return vCurrOffset;
}
vec2 parallaxOffset(vec3 viewDir,float heightScale)
{

float height=texture(bumpSampler,vBumpUV).w;
vec2 texCoordOffset=heightScale*viewDir.xy*height;
return -texCoordOffset;
}
#endif
#endif
#ifdef CLIPPLANE
in float fClipDistance;
#endif
#ifdef CLIPPLANE2
in float fClipDistance2;
#endif
#ifdef CLIPPLANE3
in float fClipDistance3;
#endif
#ifdef CLIPPLANE4
in float fClipDistance4;
#endif
#ifdef LOGARITHMICDEPTH
uniform float logarithmicDepthConstant;
in float vFragmentDepth;
#endif

#ifdef FOG
#define FOGMODE_NONE 0.
#define FOGMODE_EXP 1.
#define FOGMODE_EXP2 2.
#define FOGMODE_LINEAR 3.
#define E 2.71828
uniform vec4 vFogInfos;
uniform vec3 vFogColor;
in vec3 vFogDistance;
float CalcFogFactor()
{
float fogCoeff=1.0;
float fogStart=vFogInfos.y;
float fogEnd=vFogInfos.z;
float fogDensity=vFogInfos.w;
float fogDistance=length(vFogDistance);
if (FOGMODE_LINEAR == vFogInfos.x)
{
fogCoeff=(fogEnd-fogDistance)/(fogEnd-fogStart);
}
else if (FOGMODE_EXP == vFogInfos.x)
{
fogCoeff=1.0/pow(E,fogDistance*fogDensity);
}
else if (FOGMODE_EXP2 == vFogInfos.x)
{
fogCoeff=1.0/pow(E,fogDistance*fogDistance*fogDensity*fogDensity);
}
return clamp(fogCoeff,0.0,1.0);
}
#endif
out vec4 glFragColor;
void main(void) {
#ifdef CLIPPLANE
if (fClipDistance>0.0)
{
discard;
}
#endif
#ifdef CLIPPLANE2
if (fClipDistance2>0.0)
{
discard;
}
#endif
#ifdef CLIPPLANE3
if (fClipDistance3>0.0)
{
discard;
}
#endif
#ifdef CLIPPLANE4
if (fClipDistance4>0.0)
{
discard;
}
#endif


vec3 viewDirectionW=normalize(vEyePosition.xyz-vPositionW);
#ifdef NORMAL
vec3 normalW=normalize(vNormalW);
#else
vec3 normalW=normalize(cross(dFdx(vPositionW),dFdy(vPositionW)))*vEyePosition.w;
#endif
vec2 uvOffset=vec2(0.0,0.0);
#if defined(BUMP) || defined(PARALLAX)
#ifdef NORMALXYSCALE
float normalScale=1.0;
#else 
float normalScale=vBumpInfos.y;
#endif
#if defined(TANGENT) && defined(NORMAL)
mat3 TBN=vTBN;
#else
mat3 TBN=cotangent_frame(normalW*normalScale,vPositionW,vBumpUV);
#endif
#endif
#ifdef PARALLAX
mat3 invTBN=transposeMat3(TBN);
#ifdef PARALLAXOCCLUSION
uvOffset=parallaxOcclusion(invTBN*-viewDirectionW,invTBN*normalW,vBumpUV,vBumpInfos.z);
#else
uvOffset=parallaxOffset(invTBN*viewDirectionW,vBumpInfos.z);
#endif
#endif
#ifdef BUMP
#ifdef OBJECTSPACE_NORMALMAP
normalW=normalize(texture(bumpSampler,vBumpUV).xyz*2.0-1.0);
normalW=normalize(mat3(normalMatrix)*normalW); 
#else
normalW=perturbNormal(TBN,vBumpUV+uvOffset);
#endif
#endif
#ifdef SPECULARAA
vec3 nDfdx=dFdx(normalW.xyz);
vec3 nDfdy=dFdy(normalW.xyz);
float slopeSquare=max(dot(nDfdx,nDfdx),dot(nDfdy,nDfdy));

float geometricRoughnessFactor=pow(clamp(slopeSquare ,0.,1.),0.333);

float geometricAlphaGFactor=sqrt(slopeSquare);
#else
float geometricRoughnessFactor=0.;
#endif
#if defined(FORCENORMALFORWARD) && defined(NORMAL)
vec3 faceNormal=normalize(cross(dFdx(vPositionW),dFdy(vPositionW)))*vEyePosition.w;
#if defined(TWOSIDEDLIGHTING)
faceNormal=gl_FrontFacing ? faceNormal : -faceNormal;
#endif
normalW*=sign(dot(normalW,faceNormal));
#endif
#if defined(TWOSIDEDLIGHTING) && defined(NORMAL)
normalW=gl_FrontFacing ? normalW : -normalW;
#endif


vec3 surfaceAlbedo=vAlbedoColor.rgb;

float alpha=vAlbedoColor.a;
#ifdef ALBEDO
vec4 albedoTexture=texture(albedoSampler,vAlbedoUV+uvOffset);
#if defined(ALPHAFROMALBEDO) || defined(ALPHATEST)
alpha*=albedoTexture.a;
#endif
surfaceAlbedo*=toLinearSpace(albedoTexture.rgb);
surfaceAlbedo*=vAlbedoInfos.y;
#endif

#ifdef OPACITY
vec4 opacityMap=texture(opacitySampler,vOpacityUV+uvOffset);
#ifdef OPACITYRGB
alpha=getLuminance(opacityMap.rgb);
#else
alpha*=opacityMap.a;
#endif
alpha*=vOpacityInfos.y;
#endif
#ifdef VERTEXALPHA
alpha*=vColor.a;
#endif
#if !defined(LINKREFRACTIONTOTRANSPARENCY) && !defined(ALPHAFRESNEL)
#ifdef ALPHATEST
if (alpha<ALPHATESTVALUE)
discard;
#ifndef ALPHABLEND

alpha=1.0;
#endif
#endif
#endif
#ifdef DEPTHPREPASS
glFragColor=vec4(0.,0.,0.,1.0);
return;
#endif
#ifdef VERTEXCOLOR
surfaceAlbedo*=vColor.rgb;
#endif

vec3 ambientOcclusionColor=vec3(1.,1.,1.);
#ifdef AMBIENT
vec3 ambientOcclusionColorMap=texture(ambientSampler,vAmbientUV+uvOffset).rgb*vAmbientInfos.y;
#ifdef AMBIENTINGRAYSCALE
ambientOcclusionColorMap=vec3(ambientOcclusionColorMap.r,ambientOcclusionColorMap.r,ambientOcclusionColorMap.r);
#endif
ambientOcclusionColor=mix(ambientOcclusionColor,ambientOcclusionColorMap,vAmbientInfos.z);
#endif
#ifdef UNLIT
vec3 diffuseBase=vec3(1.,1.,1.);
#else

float microSurface=vReflectivityColor.a;
vec3 surfaceReflectivityColor=vReflectivityColor.rgb;
#ifdef METALLICWORKFLOW
vec2 metallicRoughness=surfaceReflectivityColor.rg;
#ifdef REFLECTIVITY
vec4 surfaceMetallicColorMap=texture(reflectivitySampler,vReflectivityUV+uvOffset);
#ifdef AOSTOREINMETALMAPRED
vec3 aoStoreInMetalMap=vec3(surfaceMetallicColorMap.r,surfaceMetallicColorMap.r,surfaceMetallicColorMap.r);
ambientOcclusionColor=mix(ambientOcclusionColor,aoStoreInMetalMap,vReflectivityInfos.z);
#endif
#ifdef METALLNESSSTOREINMETALMAPBLUE
metallicRoughness.r*=surfaceMetallicColorMap.b;
#else
metallicRoughness.r*=surfaceMetallicColorMap.r;
#endif
#ifdef ROUGHNESSSTOREINMETALMAPALPHA
metallicRoughness.g*=surfaceMetallicColorMap.a;
#else
#ifdef ROUGHNESSSTOREINMETALMAPGREEN
metallicRoughness.g*=surfaceMetallicColorMap.g;
#endif
#endif
#endif
#ifdef MICROSURFACEMAP
vec4 microSurfaceTexel=texture(microSurfaceSampler,vMicroSurfaceSamplerUV+uvOffset)*vMicroSurfaceSamplerInfos.y;
metallicRoughness.g*=microSurfaceTexel.r;
#endif

microSurface=1.0-metallicRoughness.g;

vec3 baseColor=surfaceAlbedo;


const vec3 DefaultSpecularReflectanceDielectric=vec3(0.04,0.04,0.04);

surfaceAlbedo=mix(baseColor.rgb*(1.0-DefaultSpecularReflectanceDielectric.r),vec3(0.,0.,0.),metallicRoughness.r);

surfaceReflectivityColor=mix(DefaultSpecularReflectanceDielectric,baseColor,metallicRoughness.r);
#else
#ifdef REFLECTIVITY
vec4 surfaceReflectivityColorMap=texture(reflectivitySampler,vReflectivityUV+uvOffset);
surfaceReflectivityColor*=toLinearSpace(surfaceReflectivityColorMap.rgb);
surfaceReflectivityColor*=vReflectivityInfos.y;
#ifdef MICROSURFACEFROMREFLECTIVITYMAP
microSurface*=surfaceReflectivityColorMap.a;
microSurface*=vReflectivityInfos.z;
#else
#ifdef MICROSURFACEAUTOMATIC
microSurface*=computeDefaultMicroSurface(microSurface,surfaceReflectivityColor);
#endif
#ifdef MICROSURFACEMAP
vec4 microSurfaceTexel=texture(microSurfaceSampler,vMicroSurfaceSamplerUV+uvOffset)*vMicroSurfaceSamplerInfos.y;
microSurface*=microSurfaceTexel.r;
#endif
#endif
#endif
#endif

microSurface=clamp(microSurface,0.,1.);

float roughness=1.-microSurface;

#ifdef ALPHAFRESNEL
#if defined(ALPHATEST) || defined(ALPHABLEND)



float opacityPerceptual=alpha;
#ifdef LINEARALPHAFRESNEL
float opacity0=opacityPerceptual;
#else
float opacity0=opacityPerceptual*opacityPerceptual;
#endif
float opacity90=fresnelGrazingReflectance(opacity0);
vec3 normalForward=faceforward(normalW,-viewDirectionW,normalW);

alpha=fresnelSchlickEnvironmentGGX(clamp(dot(viewDirectionW,normalForward),0.0,1.0),vec3(opacity0),vec3(opacity90),sqrt(microSurface)).x;
#ifdef ALPHATEST
if (alpha<ALPHATESTVALUE)
discard;
#ifndef ALPHABLEND

alpha=1.0;
#endif
#endif
#endif
#endif


float NdotVUnclamped=dot(normalW,viewDirectionW);
float NdotV=clamp(NdotVUnclamped,0.,1.)+0.00001;
float alphaG=convertRoughnessToAverageSlope(roughness);
#ifdef SPECULARAA


alphaG+=(0.75*geometricAlphaGFactor);
#endif

#ifdef REFRACTION
vec4 environmentRefraction=vec4(0.,0.,0.,0.);
vec3 refractionVector=refract(-viewDirectionW,normalW,vRefractionInfos.y);
#ifdef REFRACTIONMAP_OPPOSITEZ
refractionVector.z*=-1.0;
#endif

#ifdef REFRACTIONMAP_3D
refractionVector.y=refractionVector.y*vRefractionInfos.w;
vec3 refractionCoords=refractionVector;
refractionCoords=vec3(refractionMatrix*vec4(refractionCoords,0));
#else
vec3 vRefractionUVW=vec3(refractionMatrix*(view*vec4(vPositionW+refractionVector*vRefractionInfos.z,1.0)));
vec2 refractionCoords=vRefractionUVW.xy/vRefractionUVW.z;
refractionCoords.y=1.0-refractionCoords.y;
#endif
#ifdef LODINREFRACTIONALPHA
float refractionLOD=getLodFromAlphaG(vRefractionMicrosurfaceInfos.x,alphaG,NdotVUnclamped);
#else
float refractionLOD=getLodFromAlphaG(vRefractionMicrosurfaceInfos.x,alphaG,1.0);
#endif
#ifdef LODBASEDMICROSFURACE

refractionLOD=refractionLOD*vRefractionMicrosurfaceInfos.y+vRefractionMicrosurfaceInfos.z;
#ifdef LODINREFRACTIONALPHA









float automaticRefractionLOD=UNPACK_LOD(sampleRefraction(refractionSampler,refractionCoords).a);
float requestedRefractionLOD=max(automaticRefractionLOD,refractionLOD);
#else
float requestedRefractionLOD=refractionLOD;
#endif
environmentRefraction=sampleRefractionLod(refractionSampler,refractionCoords,requestedRefractionLOD);
#else
float lodRefractionNormalized=clamp(refractionLOD/log2(vRefractionMicrosurfaceInfos.x),0.,1.);
float lodRefractionNormalizedDoubled=lodRefractionNormalized*2.0;
vec4 environmentRefractionMid=sampleRefraction(refractionSampler,refractionCoords);
if(lodRefractionNormalizedDoubled<1.0){
environmentRefraction=mix(
sampleRefraction(refractionSamplerHigh,refractionCoords),
environmentRefractionMid,
lodRefractionNormalizedDoubled
);
}else{
environmentRefraction=mix(
environmentRefractionMid,
sampleRefraction(refractionSamplerLow,refractionCoords),
lodRefractionNormalizedDoubled-1.0
);
}
#endif
#ifdef GAMMAREFRACTION
environmentRefraction.rgb=fromRGBD(environmentRefraction);
#endif
#ifdef RGBDREFRACTION
environmentRefraction.rgb=toLinearSpace(environmentRefraction.rgb);
#endif

environmentRefraction.rgb*=vRefractionInfos.x;
#endif

#ifdef REFLECTION
vec4 environmentRadiance=vec4(0.,0.,0.,0.);
vec3 environmentIrradiance=vec3(0.,0.,0.);
vec3 reflectionVector=computeReflectionCoords(vec4(vPositionW,1.0),normalW);
#ifdef REFLECTIONMAP_OPPOSITEZ
reflectionVector.z*=-1.0;
#endif

#ifdef REFLECTIONMAP_3D
vec3 reflectionCoords=reflectionVector;
#else
vec2 reflectionCoords=reflectionVector.xy;
#ifdef REFLECTIONMAP_PROJECTION
reflectionCoords/=reflectionVector.z;
#endif
reflectionCoords.y=1.0-reflectionCoords.y;
#endif
#if defined(LODINREFLECTIONALPHA) && !defined(REFLECTIONMAP_SKYBOX)
float reflectionLOD=getLodFromAlphaG(vReflectionMicrosurfaceInfos.x,alphaG,NdotVUnclamped);
#else
float reflectionLOD=getLodFromAlphaG(vReflectionMicrosurfaceInfos.x,alphaG,1.);
#endif
#ifdef LODBASEDMICROSFURACE

reflectionLOD=reflectionLOD*vReflectionMicrosurfaceInfos.y+vReflectionMicrosurfaceInfos.z;
#ifdef LODINREFLECTIONALPHA









float automaticReflectionLOD=UNPACK_LOD(sampleReflection(reflectionSampler,reflectionCoords).a);
float requestedReflectionLOD=max(automaticReflectionLOD,reflectionLOD);
#else
float requestedReflectionLOD=reflectionLOD;
#endif
environmentRadiance=sampleReflectionLod(reflectionSampler,reflectionCoords,requestedReflectionLOD);
#else
float lodReflectionNormalized=clamp(reflectionLOD/log2(vReflectionMicrosurfaceInfos.x),0.,1.);
float lodReflectionNormalizedDoubled=lodReflectionNormalized*2.0;
vec4 environmentSpecularMid=sampleReflection(reflectionSampler,reflectionCoords);
if(lodReflectionNormalizedDoubled<1.0){
environmentRadiance=mix(
sampleReflection(reflectionSamplerHigh,reflectionCoords),
environmentSpecularMid,
lodReflectionNormalizedDoubled
);
}else{
environmentRadiance=mix(
environmentSpecularMid,
sampleReflection(reflectionSamplerLow,reflectionCoords),
lodReflectionNormalizedDoubled-1.0
);
}
#endif
#ifdef RGBDREFLECTION
environmentRadiance.rgb=fromRGBD(environmentRadiance);
#endif
#ifdef GAMMAREFLECTION
environmentRadiance.rgb=toLinearSpace(environmentRadiance.rgb);
#endif

#ifdef USESPHERICALFROMREFLECTIONMAP
#if defined(NORMAL) && defined(USESPHERICALINVERTEX)
environmentIrradiance=vEnvironmentIrradiance;
#else
vec3 irradianceVector=vec3(reflectionMatrix*vec4(normalW,0)).xyz;
#ifdef REFLECTIONMAP_OPPOSITEZ
irradianceVector.z*=-1.0;
#endif
environmentIrradiance=environmentIrradianceJones(irradianceVector);
#endif
#endif

environmentRadiance.rgb*=vReflectionInfos.x;
environmentRadiance.rgb*=vReflectionColor.rgb;
environmentIrradiance*=vReflectionColor.rgb;
#endif



float reflectance=max(max(surfaceReflectivityColor.r,surfaceReflectivityColor.g),surfaceReflectivityColor.b);
float reflectance90=fresnelGrazingReflectance(reflectance);
vec3 specularEnvironmentR0=surfaceReflectivityColor.rgb;
vec3 specularEnvironmentR90=vec3(1.0,1.0,1.0)*reflectance90;

vec3 diffuseBase=vec3(0.,0.,0.);
#ifdef SPECULARTERM
vec3 specularBase=vec3(0.,0.,0.);
#endif
#ifdef LIGHTMAP
vec3 lightmapColor=texture(lightmapSampler,vLightmapUV+uvOffset).rgb;
#ifdef GAMMALIGHTMAP
lightmapColor=toLinearSpace(lightmapColor);
#endif
lightmapColor*=vLightmapInfos.y;
#endif
lightingInfo info;
pointLightingInfo pointInfo;
spotLightingInfo spotInfo;
float shadow=1.; 
float NdotL=-1.;
#ifdef LIGHT0
#if defined(SHADOWONLY) || (defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED0) && defined(LIGHTMAPNOSPECULAR0))

#else
#ifdef PBR
#ifdef SPOTLIGHT0
spotInfo=computeSpotLightingInfo(light0.vLightData);
#ifdef LIGHT_FALLOFF_GLTF0
spotInfo.attenuation=computeDistanceLightFalloff_GLTF(spotInfo.lightDistanceSquared,light0.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff_GLTF(light0.vLightDirection.xyz,spotInfo.directionToLightCenterW,light0.vLightFalloff.z,light0.vLightFalloff.w);
#elif defined(LIGHT_FALLOFF_PHYSICAL0)
spotInfo.attenuation=computeDistanceLightFalloff_Physical(spotInfo.lightDistanceSquared);
spotInfo.attenuation*=computeDirectionalLightFalloff_Physical(light0.vLightDirection.xyz,spotInfo.directionToLightCenterW,light0.vLightDirection.w);
#elif defined(LIGHT_FALLOFF_STANDARD0)
spotInfo.attenuation=computeDistanceLightFalloff_Standard(spotInfo.lightOffset,light0.vLightFalloff.x);
spotInfo.attenuation*=computeDirectionalLightFalloff_Standard(light0.vLightDirection.xyz,spotInfo.directionToLightCenterW,light0.vLightDirection.w,light0.vLightData.w);
#else
spotInfo.attenuation=computeDistanceLightFalloff(spotInfo.lightOffset,spotInfo.lightDistanceSquared,light0.vLightFalloff.x,light0.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff(light0.vLightDirection.xyz,spotInfo.directionToLightCenterW,light0.vLightDirection.w,light0.vLightData.w,light0.vLightFalloff.z,light0.vLightFalloff.w);
#endif
info=computeSpotLighting(spotInfo,viewDirectionW,normalW,light0.vLightDirection,light0.vLightDiffuse.rgb,light0.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(POINTLIGHT0)
pointInfo=computePointLightingInfo(light0.vLightData);
#ifdef LIGHT_FALLOFF_GLTF0
pointInfo.attenuation=computeDistanceLightFalloff_GLTF(pointInfo.lightDistanceSquared,light0.vLightFalloff.y);
#elif defined(LIGHT_FALLOFF_PHYSICAL0)
pointInfo.attenuation=computeDistanceLightFalloff_Physical(pointInfo.lightDistanceSquared);
#elif defined(LIGHT_FALLOFF_STANDARD0)
pointInfo.attenuation=computeDistanceLightFalloff_Standard(pointInfo.lightOffset,light0.vLightFalloff.x);
#else
pointInfo.attenuation=computeDistanceLightFalloff(pointInfo.lightOffset,pointInfo.lightDistanceSquared,light0.vLightFalloff.x,light0.vLightFalloff.y);
#endif
info=computePointLighting(pointInfo,viewDirectionW,normalW,light0.vLightDiffuse.rgb,light0.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(HEMILIGHT0)
info=computeHemisphericLighting(viewDirectionW,normalW,light0.vLightData,light0.vLightDiffuse.rgb,light0.vLightSpecular,light0.vLightGround,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(DIRLIGHT0)
info=computeDirectionalLighting(viewDirectionW,normalW,light0.vLightData,light0.vLightDiffuse.rgb,light0.vLightSpecular,light0.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#endif
#else
#ifdef SPOTLIGHT0
info=computeSpotLighting(viewDirectionW,normalW,light0.vLightData,light0.vLightDirection,light0.vLightDiffuse.rgb,light0.vLightSpecular,light0.vLightDiffuse.a,glossiness);
#elif defined(HEMILIGHT0)
info=computeHemisphericLighting(viewDirectionW,normalW,light0.vLightData,light0.vLightDiffuse.rgb,light0.vLightSpecular,light0.vLightGround,glossiness);
#elif defined(POINTLIGHT0) || defined(DIRLIGHT0)
info=computeLighting(viewDirectionW,normalW,light0.vLightData,light0.vLightDiffuse.rgb,light0.vLightSpecular,light0.vLightDiffuse.a,glossiness);
#endif
#endif
#ifdef PROJECTEDLIGHTTEXTURE0
info.diffuse*=computeProjectionTextureDiffuseLighting(projectionLightSampler0,textureProjectionMatrix0);
#endif
#endif
#ifdef SHADOW0
#ifdef SHADOWCLOSEESM0
#if defined(SHADOWCUBE0)
shadow=computeShadowWithCloseESMCube(light0.vLightData.xyz,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.z,light0.depthValues);
#else
shadow=computeShadowWithCloseESM(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.z,light0.shadowsInfo.w);
#endif
#elif defined(SHADOWESM0)
#if defined(SHADOWCUBE0)
shadow=computeShadowWithESMCube(light0.vLightData.xyz,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.z,light0.depthValues);
#else
shadow=computeShadowWithESM(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.z,light0.shadowsInfo.w);
#endif
#elif defined(SHADOWPOISSON0)
#if defined(SHADOWCUBE0)
shadow=computeShadowWithPoissonSamplingCube(light0.vLightData.xyz,shadowSampler0,light0.shadowsInfo.y,light0.shadowsInfo.x,light0.depthValues);
#else
shadow=computeShadowWithPoissonSampling(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.y,light0.shadowsInfo.x,light0.shadowsInfo.w);
#endif
#elif defined(SHADOWPCF0)
#if defined(SHADOWLOWQUALITY0)
shadow=computeShadowWithPCF1(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY0)
shadow=computeShadowWithPCF3(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.yz,light0.shadowsInfo.x,light0.shadowsInfo.w);
#else
shadow=computeShadowWithPCF5(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.yz,light0.shadowsInfo.x,light0.shadowsInfo.w);
#endif
#elif defined(SHADOWPCSS0)
#if defined(SHADOWLOWQUALITY0)
shadow=computeShadowWithPCSS16(vPositionFromLight0,vDepthMetric0,depthSampler0,shadowSampler0,light0.shadowsInfo.y,light0.shadowsInfo.z,light0.shadowsInfo.x,light0.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY0)
shadow=computeShadowWithPCSS32(vPositionFromLight0,vDepthMetric0,depthSampler0,shadowSampler0,light0.shadowsInfo.y,light0.shadowsInfo.z,light0.shadowsInfo.x,light0.shadowsInfo.w);
#else
shadow=computeShadowWithPCSS64(vPositionFromLight0,vDepthMetric0,depthSampler0,shadowSampler0,light0.shadowsInfo.y,light0.shadowsInfo.z,light0.shadowsInfo.x,light0.shadowsInfo.w);
#endif
#else
#if defined(SHADOWCUBE0)
shadow=computeShadowCube(light0.vLightData.xyz,shadowSampler0,light0.shadowsInfo.x,light0.depthValues);
#else
shadow=computeShadow(vPositionFromLight0,vDepthMetric0,shadowSampler0,light0.shadowsInfo.x,light0.shadowsInfo.w);
#endif
#endif
#ifdef SHADOWONLY
#ifndef SHADOWINUSE
#define SHADOWINUSE
#endif
globalShadow+=shadow;
shadowLightCount+=1.0;
#endif
#else
shadow=1.;
#endif
#ifndef SHADOWONLY
#ifdef CUSTOMUSERLIGHTING
diffuseBase+=computeCustomDiffuseLighting(info,diffuseBase,shadow);
#ifdef SPECULARTERM
specularBase+=computeCustomSpecularLighting(info,specularBase,shadow);
#endif
#elif defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED0)
diffuseBase+=lightmapColor*shadow;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR0
specularBase+=info.specular*shadow*lightmapColor;
#endif
#endif
#else
diffuseBase+=info.diffuse*shadow;
#ifdef SPECULARTERM
specularBase+=info.specular*shadow;
#endif
#endif
#endif
#endif
#ifdef LIGHT1
#if defined(SHADOWONLY) || (defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED1) && defined(LIGHTMAPNOSPECULAR1))

#else
#ifdef PBR
#ifdef SPOTLIGHT1
spotInfo=computeSpotLightingInfo(light1.vLightData);
#ifdef LIGHT_FALLOFF_GLTF1
spotInfo.attenuation=computeDistanceLightFalloff_GLTF(spotInfo.lightDistanceSquared,light1.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff_GLTF(light1.vLightDirection.xyz,spotInfo.directionToLightCenterW,light1.vLightFalloff.z,light1.vLightFalloff.w);
#elif defined(LIGHT_FALLOFF_PHYSICAL1)
spotInfo.attenuation=computeDistanceLightFalloff_Physical(spotInfo.lightDistanceSquared);
spotInfo.attenuation*=computeDirectionalLightFalloff_Physical(light1.vLightDirection.xyz,spotInfo.directionToLightCenterW,light1.vLightDirection.w);
#elif defined(LIGHT_FALLOFF_STANDARD1)
spotInfo.attenuation=computeDistanceLightFalloff_Standard(spotInfo.lightOffset,light1.vLightFalloff.x);
spotInfo.attenuation*=computeDirectionalLightFalloff_Standard(light1.vLightDirection.xyz,spotInfo.directionToLightCenterW,light1.vLightDirection.w,light1.vLightData.w);
#else
spotInfo.attenuation=computeDistanceLightFalloff(spotInfo.lightOffset,spotInfo.lightDistanceSquared,light1.vLightFalloff.x,light1.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff(light1.vLightDirection.xyz,spotInfo.directionToLightCenterW,light1.vLightDirection.w,light1.vLightData.w,light1.vLightFalloff.z,light1.vLightFalloff.w);
#endif
info=computeSpotLighting(spotInfo,viewDirectionW,normalW,light1.vLightDirection,light1.vLightDiffuse.rgb,light1.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(POINTLIGHT1)
pointInfo=computePointLightingInfo(light1.vLightData);
#ifdef LIGHT_FALLOFF_GLTF1
pointInfo.attenuation=computeDistanceLightFalloff_GLTF(pointInfo.lightDistanceSquared,light1.vLightFalloff.y);
#elif defined(LIGHT_FALLOFF_PHYSICAL1)
pointInfo.attenuation=computeDistanceLightFalloff_Physical(pointInfo.lightDistanceSquared);
#elif defined(LIGHT_FALLOFF_STANDARD1)
pointInfo.attenuation=computeDistanceLightFalloff_Standard(pointInfo.lightOffset,light1.vLightFalloff.x);
#else
pointInfo.attenuation=computeDistanceLightFalloff(pointInfo.lightOffset,pointInfo.lightDistanceSquared,light1.vLightFalloff.x,light1.vLightFalloff.y);
#endif
info=computePointLighting(pointInfo,viewDirectionW,normalW,light1.vLightDiffuse.rgb,light1.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(HEMILIGHT1)
info=computeHemisphericLighting(viewDirectionW,normalW,light1.vLightData,light1.vLightDiffuse.rgb,light1.vLightSpecular,light1.vLightGround,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(DIRLIGHT1)
info=computeDirectionalLighting(viewDirectionW,normalW,light1.vLightData,light1.vLightDiffuse.rgb,light1.vLightSpecular,light1.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#endif
#else
#ifdef SPOTLIGHT1
info=computeSpotLighting(viewDirectionW,normalW,light1.vLightData,light1.vLightDirection,light1.vLightDiffuse.rgb,light1.vLightSpecular,light1.vLightDiffuse.a,glossiness);
#elif defined(HEMILIGHT1)
info=computeHemisphericLighting(viewDirectionW,normalW,light1.vLightData,light1.vLightDiffuse.rgb,light1.vLightSpecular,light1.vLightGround,glossiness);
#elif defined(POINTLIGHT1) || defined(DIRLIGHT1)
info=computeLighting(viewDirectionW,normalW,light1.vLightData,light1.vLightDiffuse.rgb,light1.vLightSpecular,light1.vLightDiffuse.a,glossiness);
#endif
#endif
#ifdef PROJECTEDLIGHTTEXTURE1
info.diffuse*=computeProjectionTextureDiffuseLighting(projectionLightSampler1,textureProjectionMatrix1);
#endif
#endif
#ifdef SHADOW1
#ifdef SHADOWCLOSEESM1
#if defined(SHADOWCUBE1)
shadow=computeShadowWithCloseESMCube(light1.vLightData.xyz,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.z,light1.depthValues);
#else
shadow=computeShadowWithCloseESM(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.z,light1.shadowsInfo.w);
#endif
#elif defined(SHADOWESM1)
#if defined(SHADOWCUBE1)
shadow=computeShadowWithESMCube(light1.vLightData.xyz,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.z,light1.depthValues);
#else
shadow=computeShadowWithESM(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.z,light1.shadowsInfo.w);
#endif
#elif defined(SHADOWPOISSON1)
#if defined(SHADOWCUBE1)
shadow=computeShadowWithPoissonSamplingCube(light1.vLightData.xyz,shadowSampler1,light1.shadowsInfo.y,light1.shadowsInfo.x,light1.depthValues);
#else
shadow=computeShadowWithPoissonSampling(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.y,light1.shadowsInfo.x,light1.shadowsInfo.w);
#endif
#elif defined(SHADOWPCF1)
#if defined(SHADOWLOWQUALITY1)
shadow=computeShadowWithPCF1(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY1)
shadow=computeShadowWithPCF3(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.yz,light1.shadowsInfo.x,light1.shadowsInfo.w);
#else
shadow=computeShadowWithPCF5(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.yz,light1.shadowsInfo.x,light1.shadowsInfo.w);
#endif
#elif defined(SHADOWPCSS1)
#if defined(SHADOWLOWQUALITY1)
shadow=computeShadowWithPCSS16(vPositionFromLight1,vDepthMetric1,depthSampler1,shadowSampler1,light1.shadowsInfo.y,light1.shadowsInfo.z,light1.shadowsInfo.x,light1.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY1)
shadow=computeShadowWithPCSS32(vPositionFromLight1,vDepthMetric1,depthSampler1,shadowSampler1,light1.shadowsInfo.y,light1.shadowsInfo.z,light1.shadowsInfo.x,light1.shadowsInfo.w);
#else
shadow=computeShadowWithPCSS64(vPositionFromLight1,vDepthMetric1,depthSampler1,shadowSampler1,light1.shadowsInfo.y,light1.shadowsInfo.z,light1.shadowsInfo.x,light1.shadowsInfo.w);
#endif
#else
#if defined(SHADOWCUBE1)
shadow=computeShadowCube(light1.vLightData.xyz,shadowSampler1,light1.shadowsInfo.x,light1.depthValues);
#else
shadow=computeShadow(vPositionFromLight1,vDepthMetric1,shadowSampler1,light1.shadowsInfo.x,light1.shadowsInfo.w);
#endif
#endif
#ifdef SHADOWONLY
#ifndef SHADOWINUSE
#define SHADOWINUSE
#endif
globalShadow+=shadow;
shadowLightCount+=1.0;
#endif
#else
shadow=1.;
#endif
#ifndef SHADOWONLY
#ifdef CUSTOMUSERLIGHTING
diffuseBase+=computeCustomDiffuseLighting(info,diffuseBase,shadow);
#ifdef SPECULARTERM
specularBase+=computeCustomSpecularLighting(info,specularBase,shadow);
#endif
#elif defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED1)
diffuseBase+=lightmapColor*shadow;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR1
specularBase+=info.specular*shadow*lightmapColor;
#endif
#endif
#else
diffuseBase+=info.diffuse*shadow;
#ifdef SPECULARTERM
specularBase+=info.specular*shadow;
#endif
#endif
#endif
#endif
#ifdef LIGHT2
#if defined(SHADOWONLY) || (defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED2) && defined(LIGHTMAPNOSPECULAR2))

#else
#ifdef PBR
#ifdef SPOTLIGHT2
spotInfo=computeSpotLightingInfo(light2.vLightData);
#ifdef LIGHT_FALLOFF_GLTF2
spotInfo.attenuation=computeDistanceLightFalloff_GLTF(spotInfo.lightDistanceSquared,light2.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff_GLTF(light2.vLightDirection.xyz,spotInfo.directionToLightCenterW,light2.vLightFalloff.z,light2.vLightFalloff.w);
#elif defined(LIGHT_FALLOFF_PHYSICAL2)
spotInfo.attenuation=computeDistanceLightFalloff_Physical(spotInfo.lightDistanceSquared);
spotInfo.attenuation*=computeDirectionalLightFalloff_Physical(light2.vLightDirection.xyz,spotInfo.directionToLightCenterW,light2.vLightDirection.w);
#elif defined(LIGHT_FALLOFF_STANDARD2)
spotInfo.attenuation=computeDistanceLightFalloff_Standard(spotInfo.lightOffset,light2.vLightFalloff.x);
spotInfo.attenuation*=computeDirectionalLightFalloff_Standard(light2.vLightDirection.xyz,spotInfo.directionToLightCenterW,light2.vLightDirection.w,light2.vLightData.w);
#else
spotInfo.attenuation=computeDistanceLightFalloff(spotInfo.lightOffset,spotInfo.lightDistanceSquared,light2.vLightFalloff.x,light2.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff(light2.vLightDirection.xyz,spotInfo.directionToLightCenterW,light2.vLightDirection.w,light2.vLightData.w,light2.vLightFalloff.z,light2.vLightFalloff.w);
#endif
info=computeSpotLighting(spotInfo,viewDirectionW,normalW,light2.vLightDirection,light2.vLightDiffuse.rgb,light2.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(POINTLIGHT2)
pointInfo=computePointLightingInfo(light2.vLightData);
#ifdef LIGHT_FALLOFF_GLTF2
pointInfo.attenuation=computeDistanceLightFalloff_GLTF(pointInfo.lightDistanceSquared,light2.vLightFalloff.y);
#elif defined(LIGHT_FALLOFF_PHYSICAL2)
pointInfo.attenuation=computeDistanceLightFalloff_Physical(pointInfo.lightDistanceSquared);
#elif defined(LIGHT_FALLOFF_STANDARD2)
pointInfo.attenuation=computeDistanceLightFalloff_Standard(pointInfo.lightOffset,light2.vLightFalloff.x);
#else
pointInfo.attenuation=computeDistanceLightFalloff(pointInfo.lightOffset,pointInfo.lightDistanceSquared,light2.vLightFalloff.x,light2.vLightFalloff.y);
#endif
info=computePointLighting(pointInfo,viewDirectionW,normalW,light2.vLightDiffuse.rgb,light2.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(HEMILIGHT2)
info=computeHemisphericLighting(viewDirectionW,normalW,light2.vLightData,light2.vLightDiffuse.rgb,light2.vLightSpecular,light2.vLightGround,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(DIRLIGHT2)
info=computeDirectionalLighting(viewDirectionW,normalW,light2.vLightData,light2.vLightDiffuse.rgb,light2.vLightSpecular,light2.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#endif
#else
#ifdef SPOTLIGHT2
info=computeSpotLighting(viewDirectionW,normalW,light2.vLightData,light2.vLightDirection,light2.vLightDiffuse.rgb,light2.vLightSpecular,light2.vLightDiffuse.a,glossiness);
#elif defined(HEMILIGHT2)
info=computeHemisphericLighting(viewDirectionW,normalW,light2.vLightData,light2.vLightDiffuse.rgb,light2.vLightSpecular,light2.vLightGround,glossiness);
#elif defined(POINTLIGHT2) || defined(DIRLIGHT2)
info=computeLighting(viewDirectionW,normalW,light2.vLightData,light2.vLightDiffuse.rgb,light2.vLightSpecular,light2.vLightDiffuse.a,glossiness);
#endif
#endif
#ifdef PROJECTEDLIGHTTEXTURE2
info.diffuse*=computeProjectionTextureDiffuseLighting(projectionLightSampler2,textureProjectionMatrix2);
#endif
#endif
#ifdef SHADOW2
#ifdef SHADOWCLOSEESM2
#if defined(SHADOWCUBE2)
shadow=computeShadowWithCloseESMCube(light2.vLightData.xyz,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.z,light2.depthValues);
#else
shadow=computeShadowWithCloseESM(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.z,light2.shadowsInfo.w);
#endif
#elif defined(SHADOWESM2)
#if defined(SHADOWCUBE2)
shadow=computeShadowWithESMCube(light2.vLightData.xyz,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.z,light2.depthValues);
#else
shadow=computeShadowWithESM(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.z,light2.shadowsInfo.w);
#endif
#elif defined(SHADOWPOISSON2)
#if defined(SHADOWCUBE2)
shadow=computeShadowWithPoissonSamplingCube(light2.vLightData.xyz,shadowSampler2,light2.shadowsInfo.y,light2.shadowsInfo.x,light2.depthValues);
#else
shadow=computeShadowWithPoissonSampling(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.y,light2.shadowsInfo.x,light2.shadowsInfo.w);
#endif
#elif defined(SHADOWPCF2)
#if defined(SHADOWLOWQUALITY2)
shadow=computeShadowWithPCF1(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY2)
shadow=computeShadowWithPCF3(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.yz,light2.shadowsInfo.x,light2.shadowsInfo.w);
#else
shadow=computeShadowWithPCF5(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.yz,light2.shadowsInfo.x,light2.shadowsInfo.w);
#endif
#elif defined(SHADOWPCSS2)
#if defined(SHADOWLOWQUALITY2)
shadow=computeShadowWithPCSS16(vPositionFromLight2,vDepthMetric2,depthSampler2,shadowSampler2,light2.shadowsInfo.y,light2.shadowsInfo.z,light2.shadowsInfo.x,light2.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY2)
shadow=computeShadowWithPCSS32(vPositionFromLight2,vDepthMetric2,depthSampler2,shadowSampler2,light2.shadowsInfo.y,light2.shadowsInfo.z,light2.shadowsInfo.x,light2.shadowsInfo.w);
#else
shadow=computeShadowWithPCSS64(vPositionFromLight2,vDepthMetric2,depthSampler2,shadowSampler2,light2.shadowsInfo.y,light2.shadowsInfo.z,light2.shadowsInfo.x,light2.shadowsInfo.w);
#endif
#else
#if defined(SHADOWCUBE2)
shadow=computeShadowCube(light2.vLightData.xyz,shadowSampler2,light2.shadowsInfo.x,light2.depthValues);
#else
shadow=computeShadow(vPositionFromLight2,vDepthMetric2,shadowSampler2,light2.shadowsInfo.x,light2.shadowsInfo.w);
#endif
#endif
#ifdef SHADOWONLY
#ifndef SHADOWINUSE
#define SHADOWINUSE
#endif
globalShadow+=shadow;
shadowLightCount+=1.0;
#endif
#else
shadow=1.;
#endif
#ifndef SHADOWONLY
#ifdef CUSTOMUSERLIGHTING
diffuseBase+=computeCustomDiffuseLighting(info,diffuseBase,shadow);
#ifdef SPECULARTERM
specularBase+=computeCustomSpecularLighting(info,specularBase,shadow);
#endif
#elif defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED2)
diffuseBase+=lightmapColor*shadow;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR2
specularBase+=info.specular*shadow*lightmapColor;
#endif
#endif
#else
diffuseBase+=info.diffuse*shadow;
#ifdef SPECULARTERM
specularBase+=info.specular*shadow;
#endif
#endif
#endif
#endif
#ifdef LIGHT3
#if defined(SHADOWONLY) || (defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED3) && defined(LIGHTMAPNOSPECULAR3))

#else
#ifdef PBR
#ifdef SPOTLIGHT3
spotInfo=computeSpotLightingInfo(light3.vLightData);
#ifdef LIGHT_FALLOFF_GLTF3
spotInfo.attenuation=computeDistanceLightFalloff_GLTF(spotInfo.lightDistanceSquared,light3.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff_GLTF(light3.vLightDirection.xyz,spotInfo.directionToLightCenterW,light3.vLightFalloff.z,light3.vLightFalloff.w);
#elif defined(LIGHT_FALLOFF_PHYSICAL3)
spotInfo.attenuation=computeDistanceLightFalloff_Physical(spotInfo.lightDistanceSquared);
spotInfo.attenuation*=computeDirectionalLightFalloff_Physical(light3.vLightDirection.xyz,spotInfo.directionToLightCenterW,light3.vLightDirection.w);
#elif defined(LIGHT_FALLOFF_STANDARD3)
spotInfo.attenuation=computeDistanceLightFalloff_Standard(spotInfo.lightOffset,light3.vLightFalloff.x);
spotInfo.attenuation*=computeDirectionalLightFalloff_Standard(light3.vLightDirection.xyz,spotInfo.directionToLightCenterW,light3.vLightDirection.w,light3.vLightData.w);
#else
spotInfo.attenuation=computeDistanceLightFalloff(spotInfo.lightOffset,spotInfo.lightDistanceSquared,light3.vLightFalloff.x,light3.vLightFalloff.y);
spotInfo.attenuation*=computeDirectionalLightFalloff(light3.vLightDirection.xyz,spotInfo.directionToLightCenterW,light3.vLightDirection.w,light3.vLightData.w,light3.vLightFalloff.z,light3.vLightFalloff.w);
#endif
info=computeSpotLighting(spotInfo,viewDirectionW,normalW,light3.vLightDirection,light3.vLightDiffuse.rgb,light3.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(POINTLIGHT3)
pointInfo=computePointLightingInfo(light3.vLightData);
#ifdef LIGHT_FALLOFF_GLTF3
pointInfo.attenuation=computeDistanceLightFalloff_GLTF(pointInfo.lightDistanceSquared,light3.vLightFalloff.y);
#elif defined(LIGHT_FALLOFF_PHYSICAL3)
pointInfo.attenuation=computeDistanceLightFalloff_Physical(pointInfo.lightDistanceSquared);
#elif defined(LIGHT_FALLOFF_STANDARD3)
pointInfo.attenuation=computeDistanceLightFalloff_Standard(pointInfo.lightOffset,light3.vLightFalloff.x);
#else
pointInfo.attenuation=computeDistanceLightFalloff(pointInfo.lightOffset,pointInfo.lightDistanceSquared,light3.vLightFalloff.x,light3.vLightFalloff.y);
#endif
info=computePointLighting(pointInfo,viewDirectionW,normalW,light3.vLightDiffuse.rgb,light3.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(HEMILIGHT3)
info=computeHemisphericLighting(viewDirectionW,normalW,light3.vLightData,light3.vLightDiffuse.rgb,light3.vLightSpecular,light3.vLightGround,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#elif defined(DIRLIGHT3)
info=computeDirectionalLighting(viewDirectionW,normalW,light3.vLightData,light3.vLightDiffuse.rgb,light3.vLightSpecular,light3.vLightDiffuse.a,roughness,NdotV,specularEnvironmentR0,specularEnvironmentR90,geometricRoughnessFactor,NdotL);
#endif
#else
#ifdef SPOTLIGHT3
info=computeSpotLighting(viewDirectionW,normalW,light3.vLightData,light3.vLightDirection,light3.vLightDiffuse.rgb,light3.vLightSpecular,light3.vLightDiffuse.a,glossiness);
#elif defined(HEMILIGHT3)
info=computeHemisphericLighting(viewDirectionW,normalW,light3.vLightData,light3.vLightDiffuse.rgb,light3.vLightSpecular,light3.vLightGround,glossiness);
#elif defined(POINTLIGHT3) || defined(DIRLIGHT3)
info=computeLighting(viewDirectionW,normalW,light3.vLightData,light3.vLightDiffuse.rgb,light3.vLightSpecular,light3.vLightDiffuse.a,glossiness);
#endif
#endif
#ifdef PROJECTEDLIGHTTEXTURE3
info.diffuse*=computeProjectionTextureDiffuseLighting(projectionLightSampler3,textureProjectionMatrix3);
#endif
#endif
#ifdef SHADOW3
#ifdef SHADOWCLOSEESM3
#if defined(SHADOWCUBE3)
shadow=computeShadowWithCloseESMCube(light3.vLightData.xyz,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.z,light3.depthValues);
#else
shadow=computeShadowWithCloseESM(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.z,light3.shadowsInfo.w);
#endif
#elif defined(SHADOWESM3)
#if defined(SHADOWCUBE3)
shadow=computeShadowWithESMCube(light3.vLightData.xyz,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.z,light3.depthValues);
#else
shadow=computeShadowWithESM(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.z,light3.shadowsInfo.w);
#endif
#elif defined(SHADOWPOISSON3)
#if defined(SHADOWCUBE3)
shadow=computeShadowWithPoissonSamplingCube(light3.vLightData.xyz,shadowSampler3,light3.shadowsInfo.y,light3.shadowsInfo.x,light3.depthValues);
#else
shadow=computeShadowWithPoissonSampling(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.y,light3.shadowsInfo.x,light3.shadowsInfo.w);
#endif
#elif defined(SHADOWPCF3)
#if defined(SHADOWLOWQUALITY3)
shadow=computeShadowWithPCF1(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY3)
shadow=computeShadowWithPCF3(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.yz,light3.shadowsInfo.x,light3.shadowsInfo.w);
#else
shadow=computeShadowWithPCF5(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.yz,light3.shadowsInfo.x,light3.shadowsInfo.w);
#endif
#elif defined(SHADOWPCSS3)
#if defined(SHADOWLOWQUALITY3)
shadow=computeShadowWithPCSS16(vPositionFromLight3,vDepthMetric3,depthSampler3,shadowSampler3,light3.shadowsInfo.y,light3.shadowsInfo.z,light3.shadowsInfo.x,light3.shadowsInfo.w);
#elif defined(SHADOWMEDIUMQUALITY3)
shadow=computeShadowWithPCSS32(vPositionFromLight3,vDepthMetric3,depthSampler3,shadowSampler3,light3.shadowsInfo.y,light3.shadowsInfo.z,light3.shadowsInfo.x,light3.shadowsInfo.w);
#else
shadow=computeShadowWithPCSS64(vPositionFromLight3,vDepthMetric3,depthSampler3,shadowSampler3,light3.shadowsInfo.y,light3.shadowsInfo.z,light3.shadowsInfo.x,light3.shadowsInfo.w);
#endif
#else
#if defined(SHADOWCUBE3)
shadow=computeShadowCube(light3.vLightData.xyz,shadowSampler3,light3.shadowsInfo.x,light3.depthValues);
#else
shadow=computeShadow(vPositionFromLight3,vDepthMetric3,shadowSampler3,light3.shadowsInfo.x,light3.shadowsInfo.w);
#endif
#endif
#ifdef SHADOWONLY
#ifndef SHADOWINUSE
#define SHADOWINUSE
#endif
globalShadow+=shadow;
shadowLightCount+=1.0;
#endif
#else
shadow=1.;
#endif
#ifndef SHADOWONLY
#ifdef CUSTOMUSERLIGHTING
diffuseBase+=computeCustomDiffuseLighting(info,diffuseBase,shadow);
#ifdef SPECULARTERM
specularBase+=computeCustomSpecularLighting(info,specularBase,shadow);
#endif
#elif defined(LIGHTMAP) && defined(LIGHTMAPEXCLUDED3)
diffuseBase+=lightmapColor*shadow;
#ifdef SPECULARTERM
#ifndef LIGHTMAPNOSPECULAR3
specularBase+=info.specular*shadow*lightmapColor;
#endif
#endif
#else
diffuseBase+=info.diffuse*shadow;
#ifdef SPECULARTERM
specularBase+=info.specular*shadow;
#endif
#endif
#endif
#endif


#if defined(ENVIRONMENTBRDF) && !defined(REFLECTIONMAP_SKYBOX)

vec2 brdfSamplerUV=vec2(NdotV,roughness);

vec4 environmentBrdf=texture(environmentBrdfSampler,brdfSamplerUV);
vec3 specularEnvironmentReflectance=specularEnvironmentR0*environmentBrdf.x+environmentBrdf.y;
#ifdef RADIANCEOCCLUSION
#ifdef AMBIENTINGRAYSCALE
float ambientMonochrome=ambientOcclusionColor.r;
#else
float ambientMonochrome=getLuminance(ambientOcclusionColor);
#endif
float seo=environmentRadianceOcclusion(ambientMonochrome,NdotVUnclamped);
specularEnvironmentReflectance*=seo;
#endif
#ifdef HORIZONOCCLUSION
#ifdef BUMP
#ifdef REFLECTIONMAP_3D
float eho=environmentHorizonOcclusion(-viewDirectionW,normalW);
specularEnvironmentReflectance*=eho;
#endif
#endif
#endif
#else

vec3 specularEnvironmentReflectance=fresnelSchlickEnvironmentGGX(NdotV,specularEnvironmentR0,specularEnvironmentR90,sqrt(microSurface));
#endif

#ifdef REFRACTION
vec3 refractance=vec3(0.0,0.0,0.0);
vec3 transmission=vec3(1.0,1.0,1.0);
#ifdef LINKREFRACTIONTOTRANSPARENCY

transmission*=(1.0-alpha);


vec3 mixedAlbedo=surfaceAlbedo;
float maxChannel=max(max(mixedAlbedo.r,mixedAlbedo.g),mixedAlbedo.b);
vec3 tint=clamp(maxChannel*mixedAlbedo,0.0,1.0);

surfaceAlbedo*=alpha;

environmentIrradiance*=alpha;

environmentRefraction.rgb*=tint;

alpha=1.0;
#endif

vec3 bounceSpecularEnvironmentReflectance=(2.0*specularEnvironmentReflectance)/(1.0+specularEnvironmentReflectance);
specularEnvironmentReflectance=mix(bounceSpecularEnvironmentReflectance,specularEnvironmentReflectance,alpha);

transmission*=1.0-specularEnvironmentReflectance;

refractance=transmission;
#endif




surfaceAlbedo.rgb=(1.-reflectance)*surfaceAlbedo.rgb;

#ifdef REFLECTION
vec3 finalIrradiance=environmentIrradiance;
finalIrradiance*=surfaceAlbedo.rgb;
#endif

#ifdef SPECULARTERM
vec3 finalSpecular=specularBase;
finalSpecular=max(finalSpecular,0.0);

vec3 finalSpecularScaled=finalSpecular*vLightingIntensity.x*vLightingIntensity.w;
#endif

#ifdef REFLECTION
vec3 finalRadiance=environmentRadiance.rgb;
finalRadiance*=specularEnvironmentReflectance;

vec3 finalRadianceScaled=finalRadiance*vLightingIntensity.z;
#endif

#ifdef REFRACTION
vec3 finalRefraction=environmentRefraction.rgb;
finalRefraction*=refractance;
#endif

#ifdef ALPHABLEND
float luminanceOverAlpha=0.0;
#if defined(REFLECTION) && defined(RADIANCEOVERALPHA)
luminanceOverAlpha+=getLuminance(finalRadianceScaled);
#endif
#if defined(SPECULARTERM) && defined(SPECULAROVERALPHA)
luminanceOverAlpha+=getLuminance(finalSpecularScaled);
#endif
#if defined(RADIANCEOVERALPHA) || defined(SPECULAROVERALPHA)
alpha=clamp(alpha+luminanceOverAlpha*luminanceOverAlpha,0.,1.);
#endif
#endif
#endif

vec3 finalDiffuse=diffuseBase;
finalDiffuse.rgb+=vAmbientColor;
finalDiffuse*=surfaceAlbedo.rgb;
finalDiffuse=max(finalDiffuse,0.0);

vec3 finalEmissive=vEmissiveColor;
#ifdef EMISSIVE
vec3 emissiveColorTex=texture(emissiveSampler,vEmissiveUV+uvOffset).rgb;
finalEmissive*=toLinearSpace(emissiveColorTex.rgb);
finalEmissive*=vEmissiveInfos.y;
#endif

#ifdef AMBIENT
vec3 ambientOcclusionForDirectDiffuse=mix(vec3(1.),ambientOcclusionColor,vAmbientInfos.w);
#else
vec3 ambientOcclusionForDirectDiffuse=ambientOcclusionColor;
#endif



vec4 finalColor=vec4(
finalDiffuse*ambientOcclusionForDirectDiffuse*vLightingIntensity.x +
#ifndef UNLIT
#ifdef REFLECTION
finalIrradiance*ambientOcclusionColor*vLightingIntensity.z +
#endif
#ifdef SPECULARTERM


finalSpecularScaled +
#endif
#ifdef REFLECTION


finalRadianceScaled +
#endif
#ifdef REFRACTION
finalRefraction*vLightingIntensity.z +
#endif
#endif
finalEmissive*vLightingIntensity.y,
alpha);

#ifdef LIGHTMAP
#ifndef LIGHTMAPEXCLUDED
#ifdef USELIGHTMAPASSHADOWMAP
finalColor.rgb*=lightmapColor;
#else
finalColor.rgb+=lightmapColor;
#endif
#endif
#endif

finalColor=max(finalColor,0.0);
#ifdef LOGARITHMICDEPTH
gl_FragDepth=log2(vFragmentDepth)*logarithmicDepthConstant*0.5;
#endif
#ifdef FOG
float fog=CalcFogFactor();
finalColor.rgb=fog*finalColor.rgb+(1.0-fog)*vFogColor;
#endif
#ifdef IMAGEPROCESSINGPOSTPROCESS


finalColor.rgb=clamp(finalColor.rgb,0.,30.0);
#else

finalColor=applyImageProcessing(finalColor);
#endif
#ifdef PREMULTIPLYALPHA

finalColor.rgb*=finalColor.a;
#endif
glFragColor=finalColor;
}