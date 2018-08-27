Shader "PBR"
{
	Properties
	{
        _MainTex ("MainTex", 2D) = "white" {}
        [NoScaleOffset] _Normal ("Normal", 2D) = "bump" {}
        [NoScaleOffset] _DetailNormal ("DetailNormal", 2D) = "bump" {}
        _DetailScale ("DetailScale", Float) = 1.0
        [NoScaleOffset] _Occlusion ("Occlusion", 2D) = "white" {}
        _OcclusionStrength ("OcclusionStrength", Range(0, 1)) = 1.0
        [NoScaleOffset] _MetallicSmoothness ("MetallicSmoothness (RA)", 2D) = "white" {}
		_Metallic ("Metallic", Range(0, 1)) = 0.0
        _Smoothness ("Smoothness", Range(0, 1)) = 0.5

        [NoScaleOffset] _Environment("Environment", Cube) = "" {}
        _LightDir("LightDir", Vector) = (0, 0, 1, 0)
        _LightColor("LightColor", Color) = (1, 1, 1, 1)
        _Ambient("Ambient", Color) = (0, 0, 0, 1)
	}
	SubShader
	{
		Pass
		{
            Tags{ "LightMode" = "ForwardBase" }

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag

			#include "UnityCG.cginc"

            sampler2D _MainTex;
            float4 _MainTex_ST;
            sampler2D _Normal;
            sampler2D _DetailNormal;
            sampler2D _Occlusion;
            sampler2D _MetallicSmoothness;

            half _DetailScale;
            half _OcclusionStrength;
            half _Metallic;
            half _Smoothness;

            samplerCUBE _Environment;
            half3 _LightDir;
            half3 _LightColor;
            half3 _Ambient;

//#define LIGHTMAP_ON
//#define SPECULARHIGHLIGHTS_OFF
//#define GLOSSYREFLECTIONS_OFF
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
				float4 uv                   : TEXCOORD0;
                float3 viewDir              : TEXCOORD1;
                float4 tangentToWorld[3]    : TEXCOORD2;
				float4 vertex               : SV_POSITION;
			};
			
			v2f vert (appdata v)
			{
                float4 posWorld = mul(unity_ObjectToWorld, v.vertex);
                float3 normalWorld = normalize(mul((float3x3) unity_ObjectToWorld, v.normal));
                float3 tangentWorld = normalize(mul((float3x3) unity_ObjectToWorld, v.tangent.xyz));
                float3 binormalWorld = cross(normalWorld, tangentWorld) * v.tangent.w * unity_WorldTransformParams.w;

                v2f o;
				o.vertex = mul(UNITY_MATRIX_VP, posWorld);
				o.uv.xy = v.uv * _MainTex_ST.xy + _MainTex_ST.zw;
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

            struct SurfaceOutputStandard
            {
                fixed3 Albedo;
                float3 Normal;
                half3 Emission;
                half Metallic;
                half Smoothness;
                half Occlusion;
                fixed Alpha;
            };

            struct Input
            {
                float2 uv_MainTex;
            };

            float3 blend_rnm(float3 n1, float3 n2)
            {
                n1.z += 1;
                n2.xy = -n2.xy;

                return n1 * dot(n1, n2) / n1.z - n2;
            }

            void surf(Input IN, inout SurfaceOutputStandard o)
            {
                fixed4 color = tex2D(_MainTex, IN.uv_MainTex);

                float3 normal = UnpackNormal(tex2D(_Normal, IN.uv_MainTex));
                float3 detailNormal = UnpackNormal(tex2D(_DetailNormal, IN.uv_MainTex * _DetailScale));
                half4 metallicSmoothness = tex2D(_MetallicSmoothness, IN.uv_MainTex);
                half occlusion = lerp(1, tex2D(_Occlusion, IN.uv_MainTex).g, _OcclusionStrength);
                half3 emission = half3(0, 0, 0);

                normal = blend_rnm(normal, detailNormal);
                half metallic = metallicSmoothness.r * _Metallic;
                half smoothness = metallicSmoothness.a * _Smoothness;

                o.Albedo = color.rgb;
                o.Normal = normal;
                o.Emission = emission;
                o.Metallic = metallic;
                o.Smoothness = smoothness;
                o.Occlusion = occlusion;
                o.Alpha = color.a;
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
                half4 bakedColorTex = UNITY_SAMPLE_TEX2D(_Lightmap, lightmapUV);
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
                half3 ambient = _Ambient;
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
                half4 rgbm = texCUBElod(_Environment, half4(R, mip));

                return DecodeHDR(rgbm, unity_SpecCube0_HDR);
            }

            half3 GISpecular(half occlusion, half perceptualRoughness, half3 reflUVW)
            {
                half3 specular = 0;

#ifdef GLOSSYREFLECTIONS_OFF
                specular = unity_IndirectSpecColor.rgb;
#else
                specular = GlossyEnvironment(perceptualRoughness, reflUVW);
#endif
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
#ifdef SPECULARHIGHLIGHTS_OFF
                specularTerm = 0.0;
#endif

                half surfaceReduction;
#ifdef COLORSPACE_GAMMA
                surfaceReduction = 1.0 - 0.28*roughness*perceptualRoughness;
#else
                surfaceReduction = 1.0 / (roughness*roughness + 1.0);
#endif
                specularTerm *= any(specColor) ? 1.0 : 0.0;

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
			
            half4 frag (v2f i) : SV_Target
			{
                Input IN;
                IN.uv_MainTex = i.uv.xy;

                SurfaceOutputStandard s;
                surf(IN, s);

                half3 tangent = i.tangentToWorld[0].xyz;
                half3 binormal = i.tangentToWorld[1].xyz;
                half3 normal = i.tangentToWorld[2].xyz;
                half3 posWorld = half3(i.tangentToWorld[0].w, i.tangentToWorld[1].w, i.tangentToWorld[2].w);
                half3 normalTangent = normalize(s.Normal);
                s.Normal = normalize(tangent * normalTangent.x + binormal * normalTangent.y + normal * normalTangent.z);

                float3 viewDir = normalize(i.viewDir);
                half3 lightDir = -normalize(_LightDir);
                half3 lightColor = _LightColor;

				return Lighting(s, viewDir, lightDir, lightColor, i.uv.zw);
			}
			ENDCG
		}
	}
}
