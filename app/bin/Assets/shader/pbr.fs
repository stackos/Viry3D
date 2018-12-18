precision highp float;
UniformBuffer(0, 1) uniform Material {
    mat4 reflectionMatrix;
    vec2 vAlbedoInfos;
    vec4 vAmbientInfos;
    vec2 vEmissiveInfos;
    vec2 vReflectionInfos;
    vec3 vBumpInfos;
    vec2 vTangentSpaceParams;
    vec3 vReflectionColor;
    vec4 vAlbedoColor;
    vec4 vLightingIntensity;
    vec3 vReflectionMicrosurfaceInfos;
    vec4 vReflectivityColor;
    vec3 vEmissiveColor;
    vec4 vEyePosition;
    vec3 vAmbientColor;
    float exposureLinear;
    float contrast;
};
UniformTexture(0, 2) uniform sampler2D albedoSampler;
UniformTexture(0, 3) uniform sampler2D ambientSampler;
UniformTexture(0, 4) uniform sampler2D emissiveSampler;
UniformTexture(0, 5) uniform sampler2D reflectivitySampler;
UniformTexture(0, 6) uniform samplerCube reflectionSampler;
UniformTexture(0, 7) uniform sampler2D environmentBrdfSampler;
UniformTexture(0, 8) uniform sampler2D bumpSampler;
Input(0) vec2 vMainUV1;
Input(1) vec3 vPositionW;
Input(2) vec3 vNormalW;
Input(3) vec3 vEnvironmentIrradiance;
Output(0) vec4 glFragColor;
vec3 computeReflectionCoords(vec4 worldPos, vec3 worldNormal) {
    vec3 viewDir = normalize(worldPos.xyz - vEyePosition.xyz);
    vec3 coords = reflect(viewDir, worldNormal);
    coords = vec3(reflectionMatrix * vec4(coords, 0));
    return coords;
}
const float PI = 3.1415926535897932384626433832795;
const float LinearEncodePowerApprox = 2.2;
const float GammaEncodePowerApprox = 1.0 / LinearEncodePowerApprox;
const vec3 LuminanceEncodeApprox = vec3(0.2126, 0.7152, 0.0722);
vec3 applyEaseInOut(vec3 x) {
    return x * x * (3.0 - 2.0 * x);
}
vec3 toLinearSpace(vec3 color) {
    return pow(color, vec3(LinearEncodePowerApprox));
}
vec3 toGammaSpace(vec3 color) {
    return pow(color, vec3(GammaEncodePowerApprox));
}
float square(float value) {
    return value * value;
}
vec4 applyImageProcessing(vec4 result) {
    result.rgb *= exposureLinear;
    const float tonemappingCalibration = 1.590579;
    result.rgb = 1.0 - exp2(-tonemappingCalibration * result.rgb);
    result.rgb = toGammaSpace(result.rgb);
    result.rgb = clamp(result.rgb, 0.0, 1.0);
    vec3 resultHighContrast = applyEaseInOut(result.rgb);
    if (contrast < 1.0) {
        result.rgb = mix(vec3(0.5, 0.5, 0.5), result.rgb, contrast);
    } else {
        result.rgb = mix(result.rgb, resultHighContrast, contrast - 1.0);
    }
    return result;
}
float convertRoughnessToAverageSlope(float roughness) {
    const float kMinimumVariance = 0.0005;
    float alphaG = square(roughness) + kMinimumVariance;
    return alphaG;
}
float computeDiffuseTerm(float NdotL, float NdotV, float VdotH, float roughness) {
    float diffuseFresnelNV = pow(clamp(1.0 - NdotL, 0.000001, 1.), 5.0);
    float diffuseFresnelNL = pow(clamp(1.0 - NdotV, 0.000001, 1.), 5.0);
    float diffuseFresnel90 = 0.5 + 2.0 * VdotH * VdotH * roughness;
    float fresnel =
        (1.0 + (diffuseFresnel90 - 1.0) * diffuseFresnelNL) *
        (1.0 + (diffuseFresnel90 - 1.0) * diffuseFresnelNV);
    return fresnel * NdotL / PI;
}
float adjustRoughnessFromLightProperties(float roughness, float lightRadius, float lightDistance) {
    float lightRoughness = lightRadius / lightDistance;
    float totalRoughness = clamp(lightRoughness + roughness, 0., 1.);
    return totalRoughness;
}
float fresnelGrazingReflectance(float reflectance0) {
    float reflectance90 = clamp(reflectance0 * 25.0, 0.0, 1.0);
    return reflectance90;
}
float getLodFromAlphaG(float cubeMapDimensionPixels, float alphaG, float NdotV) {
    float microsurfaceAverageSlope = alphaG;
    microsurfaceAverageSlope *= sqrt(abs(NdotV));
    float microsurfaceAverageSlopeTexels = microsurfaceAverageSlope * cubeMapDimensionPixels;
    float lod = log2(microsurfaceAverageSlopeTexels);
    return lod;
}
float environmentRadianceOcclusion(float ambientOcclusion, float NdotVUnclamped) {
    float temp = NdotVUnclamped + ambientOcclusion;
    return clamp(square(temp) - 1.0 + ambientOcclusion, 0.0, 1.0);
}
float environmentHorizonOcclusion(vec3 view, vec3 normal) {
    vec3 reflection = reflect(view, normal);
    float temp = clamp(1.0 + 1.1 * dot(reflection, normal), 0.0, 1.0);
    return square(temp);
}
mat3 cotangent_frame(vec3 normal, vec3 p, vec2 uv) {
    uv = gl_FrontFacing ? uv : -uv;
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
    vec3 dp2perp = cross(dp2, normal);
    vec3 dp1perp = cross(normal, dp1);
    vec3 tangent = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 bitangent = dp2perp * duv1.y + dp1perp * duv2.y;
    tangent *= vTangentSpaceParams.x;
    bitangent *= vTangentSpaceParams.y;
    float invmax = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    return mat3(tangent * invmax, bitangent * invmax, normal);
}
vec3 perturbNormal(mat3 cotangentFrame, vec2 uv) {
    vec3 map = texture(bumpSampler, uv).xyz;
    map = map * 2.0 - 1.0;
    map = normalize(map * vec3(vBumpInfos.y, vBumpInfos.y, 1.0));
    return normalize(cotangentFrame * map);
}
void main(void) {
    vec3 viewDirectionW = normalize(vEyePosition.xyz - vPositionW);
    vec3 normalW = normalize(vNormalW);
    vec2 uvOffset = vec2(0.0, 0.0);
    float normalScale = 1.0;
    mat3 TBN = cotangent_frame(normalW * normalScale, vPositionW, vMainUV1);
    normalW = perturbNormal(TBN, vMainUV1 + uvOffset);
    vec3 nDfdx = dFdx(normalW.xyz);
    vec3 nDfdy = dFdy(normalW.xyz);
    float slopeSquare = max(dot(nDfdx, nDfdx), dot(nDfdy, nDfdy));
    float geometricRoughnessFactor = pow(clamp(slopeSquare, 0., 1.), 0.333);
    float geometricAlphaGFactor = sqrt(slopeSquare);
    vec3 surfaceAlbedo = vAlbedoColor.rgb;
    float alpha = vAlbedoColor.a;
    vec4 albedoTexture = texture(albedoSampler, vMainUV1 + uvOffset);
    surfaceAlbedo *= toLinearSpace(albedoTexture.rgb);
    surfaceAlbedo *= vAlbedoInfos.y;
    vec3 ambientOcclusionColor = vec3(1., 1., 1.);
    vec3 ambientOcclusionColorMap = texture(ambientSampler, vMainUV1 + uvOffset).rgb * vAmbientInfos.y;
    ambientOcclusionColorMap = vec3(ambientOcclusionColorMap.r, ambientOcclusionColorMap.r, ambientOcclusionColorMap.r);
    ambientOcclusionColor = mix(ambientOcclusionColor, ambientOcclusionColorMap, vAmbientInfos.z);
    float microSurface = vReflectivityColor.a;
    vec3 surfaceReflectivityColor = vReflectivityColor.rgb;
    vec2 metallicRoughness = surfaceReflectivityColor.rg;
    vec4 surfaceMetallicColorMap = texture(reflectivitySampler, vMainUV1 + uvOffset);
    metallicRoughness.r *= surfaceMetallicColorMap.b;
    metallicRoughness.g *= surfaceMetallicColorMap.g;
    microSurface = 1.0 - metallicRoughness.g;
    vec3 baseColor = surfaceAlbedo;
    const vec3 DefaultSpecularReflectanceDielectric = vec3(0.04, 0.04, 0.04);
    surfaceAlbedo = mix(baseColor.rgb * (1.0 - DefaultSpecularReflectanceDielectric.r), vec3(0., 0., 0.), metallicRoughness.r);
    surfaceReflectivityColor = mix(DefaultSpecularReflectanceDielectric, baseColor, metallicRoughness.r);
    microSurface = clamp(microSurface, 0., 1.);
    float roughness = 1. - microSurface;
    float NdotVUnclamped = dot(normalW, viewDirectionW);
    float NdotV = clamp(NdotVUnclamped, 0., 1.) + 0.00001;
    float alphaG = convertRoughnessToAverageSlope(roughness);
    alphaG += (0.75 * geometricAlphaGFactor);
    vec4 environmentRadiance = vec4(0., 0., 0., 0.);
    vec3 environmentIrradiance = vec3(0., 0., 0.);
    vec3 reflectionVector = computeReflectionCoords(vec4(vPositionW, 1.0), normalW);
    vec3 reflectionCoords = reflectionVector;
    float reflectionLOD = getLodFromAlphaG(vReflectionMicrosurfaceInfos.x, alphaG, 1.);
    reflectionLOD = reflectionLOD * vReflectionMicrosurfaceInfos.y + vReflectionMicrosurfaceInfos.z;
    float requestedReflectionLOD = reflectionLOD;
    environmentRadiance = textureLod(reflectionSampler, reflectionCoords, requestedReflectionLOD);
    environmentIrradiance = vEnvironmentIrradiance;
    environmentRadiance.rgb *= vReflectionInfos.x;
    environmentRadiance.rgb *= vReflectionColor.rgb;
    environmentIrradiance *= vReflectionColor.rgb;
    float reflectance = max(max(surfaceReflectivityColor.r, surfaceReflectivityColor.g), surfaceReflectivityColor.b);
    float reflectance90 = fresnelGrazingReflectance(reflectance);
    vec3 specularEnvironmentR0 = surfaceReflectivityColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;
    vec3 diffuseBase = vec3(0., 0., 0.);
    float shadow = 1.;
    float NdotL = -1.;
    vec2 brdfSamplerUV = vec2(NdotV, roughness);
    vec4 environmentBrdf = texture(environmentBrdfSampler, brdfSamplerUV);
    vec3 specularEnvironmentReflectance = specularEnvironmentR0 * environmentBrdf.x + environmentBrdf.y;
    float ambientMonochrome = ambientOcclusionColor.r;
    float seo = environmentRadianceOcclusion(ambientMonochrome, NdotVUnclamped);
    specularEnvironmentReflectance *= seo;
    float eho = environmentHorizonOcclusion(-viewDirectionW, normalW);
    specularEnvironmentReflectance *= eho;
    surfaceAlbedo.rgb = (1. - reflectance) * surfaceAlbedo.rgb;
    vec3 finalIrradiance = environmentIrradiance;
    finalIrradiance *= surfaceAlbedo.rgb;
    vec3 finalRadiance = environmentRadiance.rgb;
    finalRadiance *= specularEnvironmentReflectance;
    vec3 finalRadianceScaled = finalRadiance * vLightingIntensity.z;
    vec3 finalDiffuse = diffuseBase;
    finalDiffuse.rgb += vAmbientColor;
    finalDiffuse *= surfaceAlbedo.rgb;
    finalDiffuse = max(finalDiffuse, 0.0);
    vec3 finalEmissive = vEmissiveColor;
    vec3 emissiveColorTex = texture(emissiveSampler, vMainUV1 + uvOffset).rgb;
    finalEmissive *= toLinearSpace(emissiveColorTex.rgb);
    finalEmissive *= vEmissiveInfos.y;
    vec3 ambientOcclusionForDirectDiffuse = mix(vec3(1.), ambientOcclusionColor, vAmbientInfos.w);
    vec4 finalColor = vec4(
        finalDiffuse * ambientOcclusionForDirectDiffuse * vLightingIntensity.x +
        finalIrradiance * ambientOcclusionColor * vLightingIntensity.z +
        finalRadianceScaled +
        finalEmissive * vLightingIntensity.y,
        alpha);
    finalColor = max(finalColor, 0.0);
    finalColor = applyImageProcessing(finalColor);
    glFragColor = finalColor;
}