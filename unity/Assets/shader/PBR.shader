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
        [NoScaleOffset] _MetallicSmoothness ("MetallicSmoothness", 2D) = "white" {}
		_Metallic ("Metallic", Range(0, 1)) = 0.0
        _Smoothness ("Smoothness", Range(0, 1)) = 0.5
	}
	SubShader
	{
		Pass
		{
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

            fixed4 _LightColor0;

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
                float3 eyeVec               : TEXCOORD1;
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
                o.eyeVec = posWorld.xyz - _WorldSpaceCameraPos;
                o.uv.zw = v.uv1 * unity_LightmapST.xy + unity_LightmapST.zw;

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
                half occlusion = lerp(1, tex2D(_Occlusion, IN.uv_MainTex), _OcclusionStrength);
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
                half oneMinusDielectricSpec = unity_ColorSpaceDielectricSpec.a;
                return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
            }

            half3 DiffuseAndSpecularFromMetallic(half3 albedo, half metallic, out half3 specColor, out half oneMinusReflectivity)
            {
                specColor = lerp(unity_ColorSpaceDielectricSpec.rgb, albedo, metallic);
                oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
                return albedo * oneMinusReflectivity;
            }

            half4 BRDF_PBS(half3 diffColor, half3 specColor, half oneMinusReflectivity, half smoothness, float3 normal, float3 viewDir)
            {
                return half4(diffColor, 1);
            }

            half4 Lighting(SurfaceOutputStandard s, float3 viewDir)
            {
                s.Normal = normalize(s.Normal);

                half oneMinusReflectivity;
                half3 specColor;
                s.Albedo = DiffuseAndSpecularFromMetallic(s.Albedo, s.Metallic, specColor, oneMinusReflectivity);

                half4 c = BRDF_PBS(s.Albedo, specColor, oneMinusReflectivity, s.Smoothness, s.Normal, viewDir);
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

                half3 lightColor = _LightColor0.rgb;
                half3 lightDir = _WorldSpaceLightPos0.xyz;
                float3 viewDir = -normalize(i.eyeVec);

				return Lighting(s, viewDir);
			}
			ENDCG
		}
	}
}
