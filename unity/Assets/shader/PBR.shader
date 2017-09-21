Shader "PBR"
{
	Properties
	{
		_Color ("Color", Color) = (1, 1, 1, 1)
		_MainTex ("Texture", 2D) = "white" {}
		_Normal ("Normal", 2D) = "bump" {}
		_CubeMap ("CubeMap", CUBE) = "" {}
		_SpecMap ("SpecMap", 2D) = "white" {}
		_Spec ("Spec", Color) = (1, 1, 1, 1)
		_Smoothness ("Smoothness", Range(0, 1)) = 0.5
		_EmissiveMap("Emissive Map", 2D) = "white" {}
		_Emissive("Emissive", Color) = (0, 0, 0, 0)
		_LightDir ("LightDir", Vector) = (0, 0, 1, 0)
		_LightColor ("LightColor", Color) = (1, 1, 1, 1)
	}

	SubShader
	{
		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			
			#include "UnityCG.cginc"

			struct appdata
			{
				float4 vertex : POSITION;
				float2 uv : TEXCOORD0;
				float3 normal : NORMAL;
				float4 tangent : TANGENT;
			};

			struct v2f
			{
				float4 pos : SV_POSITION;
				float2 uv : TEXCOORD0;
				half3 tspace0 : TEXCOORD1;
				half3 tspace1 : TEXCOORD2;
				half3 tspace2 : TEXCOORD3;
				float3 posWorld : TEXCOORD4;
			};

			//sampler2D unity_NHxRoughness;

			fixed4 _Color;
			sampler2D _MainTex;
			float4 _MainTex_ST;
			sampler2D _Normal;

			samplerCUBE _CubeMap;
			sampler2D _SpecMap;
			fixed3 _Spec;
			half _Smoothness;
			sampler2D _EmissiveMap;
			fixed3 _Emissive;

			half3 _LightDir;
			fixed4 _LightColor;

			v2f vert (appdata v)
			{
				v2f o;
				o.pos = UnityObjectToClipPos(v.vertex);
				o.uv = TRANSFORM_TEX(v.uv, _MainTex);

				o.posWorld = mul(unity_ObjectToWorld, v.vertex).xyz;

				half3 normal = UnityObjectToWorldNormal(v.normal);
                half3 tangent = UnityObjectToWorldDir(v.tangent.xyz);
				half tangentSign = v.tangent.w * unity_WorldTransformParams.w;
				half3 bitangent = cross(normal, tangent) * tangentSign;

				o.tspace0 = half3(tangent.x, bitangent.x, normal.x);
                o.tspace1 = half3(tangent.y, bitangent.y, normal.y);
                o.tspace2 = half3(tangent.z, bitangent.z, normal.z);

				return o;
			}

			inline half2 Pow4 (half2 x) { return x*x*x*x; }

			inline half3 Unity_SafeNormalize(half3 inVec)
			{
				half dp3 = max(0.001f, dot(inVec, inVec));
				return inVec * rsqrt(dp3);
			}

			fixed4 frag (v2f i) : SV_Target
			{
				fixed4 base = tex2D(_MainTex, i.uv);
				fixed4 c = base * _Color;
				fixed3 n = UnpackNormal(tex2D(_Normal, i.uv));

				half3 normal;
                normal.x = dot(i.tspace0, n);
                normal.y = dot(i.tspace1, n);
                normal.z = dot(i.tspace2, n);

				normal = normalize(normal);
				half3 viewDir = normalize(UnityWorldSpaceViewDir(i.posWorld));
				half3 lightDir = normalize(-_LightDir);
				fixed3 lightColor = _LightColor;

				fixed4 specMap = tex2D(_SpecMap, i.uv);
				fixed3 specColor = _Spec * specMap.rgb;
				half smoothness = _Smoothness * specMap.a;
				half roughness = 1 - smoothness;
				
				half reflectivity = max(max(specColor.r, specColor.g), specColor.b);
				half oneMinusReflectivity = 1 - reflectivity;

				half3 reflDir = reflect(viewDir, normal);
				half nl = saturate(dot(normal, lightDir));
				half nv = saturate(dot(normal, viewDir));

				half2 rlPow4AndFresnelTerm = Pow4(half2(dot(reflDir, lightDir), 1 - nv));
				half rlPow4 = rlPow4AndFresnelTerm.x;
				half fresnelTerm = rlPow4AndFresnelTerm.y;
				half grazingTerm = saturate(smoothness + reflectivity);

#if 0
				half LUT_RANGE = 16.0;
				half specular = tex2D(unity_NHxRoughness, half2(rlPow4, 1 - smoothness)).UNITY_ATTEN_CHANNEL * LUT_RANGE;
#else
				half3 halfDir = Unity_SafeNormalize(lightDir + viewDir);
				half lh = saturate(dot(lightDir, halfDir));
				half nh = saturate(dot(normal, halfDir));
				half roughness2 = roughness * roughness;
				half a = roughness2;
				half a2 = a*a;
				half d = nh * nh * (a2 - 1.h) + 1.00001h;
				half specular = a2 / (max(0.1h, lh*lh) * (roughness2 + 0.5h) * (d * d) * 4);
				specular = specular - 1e-4h;
#endif

				fixed3 ambient = UNITY_LIGHTMODEL_AMBIENT * c.rgb;
				fixed3 diffuse = ambient + lightColor * nl * c.rgb;
				fixed3 spec = lightColor * nl * specular * specColor;

				half3 reflUVW = reflect(-viewDir, normal);
				
				int UNITY_SPECCUBE_LOD_STEPS = 6;
				half mip = roughness * (1.7 - 0.7 * roughness) * UNITY_SPECCUBE_LOD_STEPS;
				fixed3 env = texCUBElod(_CubeMap, half4(reflUVW, mip)).rgb;
				fixed3 gi = env * lerp(specColor, grazingTerm, fresnelTerm);

				c.rgb = diffuse * oneMinusReflectivity + gi + spec;

				fixed3 emissiveColor = _Emissive * tex2D(_EmissiveMap, i.uv).rgb;
				c.rgb += emissiveColor;

				return c;
			}
			ENDCG
		}
	}
}
