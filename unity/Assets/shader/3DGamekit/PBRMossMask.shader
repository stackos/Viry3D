Shader "3DGamekit/PBRMossMask"
{
	Properties
	{
        _MainTex ("MainTex", 2D) = "white" {}
        [NoScaleOffset] _Normal ("Normal", 2D) = "bump" {}
        [NoScaleOffset] _DetailNormal ("DetailNormal", 2D) = "bump" {}
        _DetailScale ("DetailScale", Float) = 1.0
        [NoScaleOffset] _MetallicSmoothness ("MetallicSmoothness (RA)", 2D) = "white" {}
		_Metallic ("Metallic", Range(0, 1)) = 0.0
        _Smoothness ("Smoothness", Range(0, 1)) = 0.5
        [NoScaleOffset] _Occlusion ("Occlusion", 2D) = "white" {}
        _OcclusionStrength ("OcclusionStrength", Range(0, 1)) = 1.0
        [NoScaleOffset] _Emission ("Emission", 2D) = "white" {}
        _EmissionColor ("EmissionColor", Color) = (0, 0, 0, 1)

        [NoScaleOffset] _Environment("Environment", Cube) = "" {}
        _Ambient("Ambient", Color) = (0, 0, 0, 1)
        _LightDir("LightDir", Vector) = (0, 0, 1, 0)
        _LightColor("LightColor", Color) = (1, 1, 1, 1)

        _TopAlbedo ("TopAlbedo", 2D) = "white" {}
        [NoScaleOffset] _TopNormal ("TopNormal", 2D) = "bump" {}
        _TopDetailNormal ("TopDetailNormal", 2D) = "bump" {}
        [NoScaleOffset] _TopMetallicSmoothness ("TopMetallicSmoothness (RA)", 2D) = "white" {}
		_TopMetallic ("TopMetallic", Range(0, 1)) = 0.0
        _TopSmoothness ("TopSmoothness", Range(0, 1)) = 0.5
	}
	SubShader
	{
		Pass
		{
            Tags{ "LightMode" = "ForwardBase" }

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
            #define surface_input surf_in
            #define surface surf

			#include "UnityCG.cginc"

            struct Input
            {
                float2 uv;
                float2 uv_TopAlbedo;
                float2 uv_TopDetailNormal;
            };

            #include "../PBR.cginc"

            float4 _MainTex_ST;
            sampler2D _DetailNormal;
            half _DetailScale;
            sampler2D _TopAlbedo;
            float4 _TopAlbedo_ST;
            sampler2D _TopNormal;
            sampler2D _TopDetailNormal;
            float4 _TopDetailNormal_ST;
            sampler2D _TopMetallicSmoothness;
            half _TopMetallic;
            half _TopSmoothness;

            void surf_in(v2f i, inout Input IN)
            {
                IN.uv = i.uv.xy * _MainTex_ST.xy + _MainTex_ST.zw;
                IN.uv_TopAlbedo = i.uv.xy * _TopAlbedo_ST.xy + _TopAlbedo_ST.zw;
                IN.uv_TopDetailNormal = i.uv.xy * _TopDetailNormal_ST.xy + _TopDetailNormal_ST.zw;
            }

            float3 blend_rnm(float3 n1, float3 n2)
            {
                n1.z += 1;
                n2.xy = -n2.xy;

                return n1 * dot(n1, n2) / n1.z - n2;
            }

            void surf(Input IN, inout SurfaceOutputStandard o)
            {
                fixed4 albedo = tex2D(_MainTex, IN.uv);
                float3 normal = UnpackNormal(tex2D(_Normal, IN.uv));
                float3 detailNormal = UnpackNormal(tex2D(_DetailNormal, IN.uv * _DetailScale));
                normal = blend_rnm(normal, detailNormal);
                half4 metallicSmoothness = tex2D(_MetallicSmoothness, IN.uv);
                half metallic = metallicSmoothness.r * _Metallic;
                half smoothness = metallicSmoothness.a * _Smoothness;
                half occlusion = lerp(1, tex2D(_Occlusion, IN.uv).g, _OcclusionStrength);
                half3 emission = tex2D(_Emission, IN.uv) * _EmissionColor;

                half mask = albedo.a;
                fixed4 albedoTop = tex2D(_TopAlbedo, IN.uv_TopAlbedo);
                albedo = lerp(albedo, albedoTop * occlusion, mask);
                float3 normalTop = UnpackNormal(tex2D(_TopNormal, IN.uv));
                float3 detailNormalTop = UnpackNormal(tex2D(_TopDetailNormal, IN.uv_TopDetailNormal));
                normalTop = blend_rnm(normalTop, normal);
                normalTop = blend_rnm(normalTop, detailNormalTop);
                normal = lerp(normal, normalTop, mask);
                half4 metallicSmoothnessTop = tex2D(_MetallicSmoothness, IN.uv);
                metallic = lerp(metallic, metallicSmoothnessTop.r * _TopMetallic, mask);
                smoothness = lerp(smoothness, metallicSmoothnessTop.a * _TopSmoothness, mask);

                o.Albedo = albedo.rgb;
                o.Normal = normal;
                o.Metallic = metallic;
                o.Smoothness = smoothness;
                o.Occlusion = occlusion;
                o.Emission = emission;
                o.Alpha = 1.0;
            }
			ENDCG
		}
	}
}
