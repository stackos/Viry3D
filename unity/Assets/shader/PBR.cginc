#ifndef PBR_INCLUDE
#define PBR_INCLUDE

samplerCUBE u_environment;
half3 u_ambient;
half3 u_lightDir;
half3 u_lightColor;

//#define LIGHTMAP_ON
//#define SH_ON
#define COLORSPACE_GAMMA

#ifdef COLORSPACE_GAMMA
#define ColorSpaceDielectricSpec half4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else
#define ColorSpaceDielectricSpec half4(0.04, 0.04, 0.04, 1.0 - 0.04)
#endif
#define PI 3.14159265359f
#define INV_PI 0.31830988618f
#define SPECCUBE_LOD_STEPS 6

struct appdata
{
    float4 vertex   : POSITION;
    half3 normal    : NORMAL;
    float2 uv       : TEXCOORD0;
    float2 uv1      : TEXCOORD1;
    half4 tangent   : TANGENT;
};

struct v2f
{
    float4 vertex               : SV_POSITION;
    float4 uv                   : TEXCOORD0;
    float3 viewDir              : TEXCOORD1;
    float4 tangentToWorld[3]    : TEXCOORD2;
};

struct SurfaceOutputStandard
{
    fixed3 Albedo;
    float3 Normal;
    half Metallic;
    half Smoothness;
    half Occlusion;
    half3 Emission;
    fixed Alpha;
};

v2f vert(appdata v)
{
    float4 posWorld = mul(unity_ObjectToWorld, v.vertex);
    float3 normalWorld = normalize(mul((float3x3) unity_ObjectToWorld, v.normal));
    float3 tangentWorld = normalize(mul((float3x3) unity_ObjectToWorld, v.tangent.xyz));
    float3 binormalWorld = cross(normalWorld, tangentWorld) * v.tangent.w * unity_WorldTransformParams.w;

    v2f o;
    o.vertex = mul(UNITY_MATRIX_VP, posWorld);
    o.uv.xy = v.uv;
    o.tangentToWorld[0].xyz = tangentWorld;
    o.tangentToWorld[1].xyz = binormalWorld;
    o.tangentToWorld[2].xyz = normalWorld;
    o.tangentToWorld[0].w = posWorld.x;
    o.tangentToWorld[1].w = posWorld.y;
    o.tangentToWorld[2].w = posWorld.z;
    o.viewDir = _WorldSpaceCameraPos - posWorld.xyz;

#ifdef LIGHTMAP_ON
    o.uv.zw = v.uv1 * unity_LightmapST.xy + unity_LightmapST.zw;
#else
    o.uv.zw = 0;
#endif

    return o;
}

half OneMinusReflectivityFromMetallic(half metallic)
{
    half oneMinusDielectricSpec = ColorSpaceDielectricSpec.a;
    return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}

half3 DiffuseAndSpecularFromMetallic(half3 albedo, half metallic, out half3 specColor, out half oneMinusReflectivity)
{
    specColor = lerp(ColorSpaceDielectricSpec.rgb, albedo, metallic);
    oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
    return albedo * oneMinusReflectivity;
}

half Pow5(half x)
{
    return x * x * x*x * x;
}

half DisneyDiffuse(half NdotV, half NdotL, half LdotH, half perceptualRoughness)
{
    half fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    half lightScatter = (1 + (fd90 - 1) * Pow5(1 - NdotL));
    half viewScatter = (1 + (fd90 - 1) * Pow5(1 - NdotV));

    return lightScatter * viewScatter;
}

float PerceptualRoughnessToRoughness(float perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

half SmithJointGGXVisibilityTerm(half NdotL, half NdotV, half roughness)
{
    half a = roughness;
    half lambdaV = NdotL * (NdotV * (1 - a) + a);
    half lambdaL = NdotV * (NdotL * (1 - a) + a);

    return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

float GGXTerm(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0f;
    return INV_PI * a2 / (d * d + 1e-7f);
}

half3 FresnelTerm(half3 F0, half cosA)
{
    half t = Pow5(1 - cosA);
    return F0 + (1 - F0) * t;
}

half3 FresnelLerp(half3 F0, half3 F90, half cosA)
{
    half t = Pow5(1 - cosA);
    return lerp(F0, F90, t);
}

half3 GIDiffuse(half occlusion, float2 lightmapUV, half3 normal)
{
    half3 diffuse = 0;

#ifdef LIGHTMAP_ON
    half4 bakedColorTex = tex2D(_Lightmap, lightmapUV);
    half3 bakedColor = DecodeLightmap(bakedColorTex);
    diffuse = bakedColor;
#else
    #ifdef SH_ON
        half3 ambient = SHEvalLinearL0L1(half4(normal, 1.0));
        ambient += SHEvalLinearL2(half4(normal, 1.0));
        ambient = max(half3(0, 0, 0), ambient);

        #ifdef COLORSPACE_GAMMA
            ambient = LinearToGammaSpace(ambient);
        #endif
    #else
        half3 ambient = u_ambient;
    #endif

    diffuse = ambient;
#endif

    diffuse *= occlusion;

    return diffuse;
}

float SmoothnessToPerceptualRoughness(float smoothness)
{
    return (1 - smoothness);
}

half perceptualRoughnessToMipmapLevel(half perceptualRoughness)
{
    return perceptualRoughness * SPECCUBE_LOD_STEPS;
}

half3 GlossyEnvironment(half perceptualRoughness, half3 reflUVW)
{
    perceptualRoughness = perceptualRoughness * (1.7 - 0.7*perceptualRoughness);

    half mip = perceptualRoughnessToMipmapLevel(perceptualRoughness);
    half3 R = reflUVW;
    half4 rgbm = texCUBElod(u_environment, half4(R, mip));

    return DecodeHDR(rgbm, unity_SpecCube0_HDR);
}

half3 GISpecular(half occlusion, half perceptualRoughness, half3 reflUVW)
{
    half3 specular = 0;

    specular = GlossyEnvironment(perceptualRoughness, reflUVW);
    specular = specular * occlusion;

    return specular;
}

half4 BRDF_PBS(half3 diffColor, half3 specColor, half oneMinusReflectivity, half smoothness, float3 normal, float3 viewDir, float3 lightDir, half3 lightColor, half3 giDiffuse, half3 giSpecular)
{
    float perceptualRoughness = SmoothnessToPerceptualRoughness(smoothness);
    float3 halfDir = normalize(lightDir + viewDir);
    half nv = abs(dot(normal, viewDir));
    half nl = saturate(dot(normal, lightDir));
    float nh = saturate(dot(normal, halfDir));
    half lv = saturate(dot(lightDir, viewDir));
    half lh = saturate(dot(lightDir, halfDir));

    half diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl;

    float roughness = PerceptualRoughnessToRoughness(perceptualRoughness);
    roughness = max(roughness, 0.002);
    half V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
    float D = GGXTerm(nh, roughness);
    half specularTerm = V * D * PI;

#ifdef COLORSPACE_GAMMA
    specularTerm = sqrt(max(1e-4h, specularTerm));
#endif
    specularTerm = max(0, specularTerm * nl);
    specularTerm *= any(specColor) ? 1.0 : 0.0;

    half surfaceReduction;
#ifdef COLORSPACE_GAMMA
    surfaceReduction = 1.0 - 0.28*roughness*perceptualRoughness;
#else
    surfaceReduction = 1.0 / (roughness*roughness + 1.0);
#endif

    half grazingTerm = saturate(smoothness + (1 - oneMinusReflectivity));
    half3 color = diffColor * (giDiffuse + lightColor * diffuseTerm)
        + specularTerm * lightColor * FresnelTerm(specColor, lh)
        + surfaceReduction * giSpecular * FresnelLerp(specColor, grazingTerm, nv);

    return half4(color, 1);
}

half4 Lighting(SurfaceOutputStandard s, float3 viewDir, float3 lightDir, half3 lightColor, float2 lightmapUV)
{
    half oneMinusReflectivity;
    half3 specColor;
    s.Albedo = DiffuseAndSpecularFromMetallic(s.Albedo, s.Metallic, specColor, oneMinusReflectivity);

    half perceptualRoughness = SmoothnessToPerceptualRoughness(s.Smoothness);
    half3 reflUVW = reflect(-viewDir, s.Normal);

    half3 giDiffuse = GIDiffuse(s.Occlusion, lightmapUV, s.Normal);
    half3 giSpecular = GISpecular(s.Occlusion, perceptualRoughness, reflUVW);

    half4 c = BRDF_PBS(s.Albedo, specColor, oneMinusReflectivity, s.Smoothness, s.Normal, viewDir, lightDir, lightColor, giDiffuse, giSpecular);
    c.rgb += s.Emission;
    c.a = s.Alpha;

    return c;
}

void surface_input(v2f i, inout Input IN);
void surface(Input IN, inout SurfaceOutputStandard o);

half4 frag (v2f i) : SV_Target
{
    Input IN;
    surface_input(i, IN);

    SurfaceOutputStandard s;
    surface(IN, s);

    half3 tangent = i.tangentToWorld[0].xyz;
    half3 binormal = i.tangentToWorld[1].xyz;
    half3 normal = i.tangentToWorld[2].xyz;
    half3 posWorld = half3(i.tangentToWorld[0].w, i.tangentToWorld[1].w, i.tangentToWorld[2].w);
    half3 normalTangent = normalize(s.Normal);
    s.Normal = normalize(tangent * normalTangent.x + binormal * normalTangent.y + normal * normalTangent.z);

    float3 viewDir = normalize(i.viewDir);
    half3 lightDir = -normalize(u_lightDir);
    half3 lightColor = u_lightColor;

	return Lighting(s, viewDir, lightDir, lightColor, i.uv.zw);
}

#endif
