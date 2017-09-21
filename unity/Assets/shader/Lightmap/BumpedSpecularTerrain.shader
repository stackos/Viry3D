Shader "Lightmap/BumpedSpecularTerrain"
{
	Properties
	{
		_Color("Color", Color) = (1, 1, 1, 1)
		_Splat0 ("Layer 0", 2D) = "white" {}
		_Normal0 ("Normal 0", 2D) = "bump" {}
		_Splat1 ("Layer 1", 2D) = "white" {}
		_Normal1 ("Normal 1", 2D) = "bump" {}
		_Splat2 ("Layer 2", 2D) = "white" {}
		_Normal2 ("Normal 2", 2D) = "bump" {}
		_Control ("Control (RGBA)", 2D) = "white" {}
		_Spec0 ("Spec 0", Color) = (1, 1, 1, 1)
		_Spec1 ("Spec 1", Color) = (1, 1, 1, 1)
		_Spec2 ("Spec 2", Color) = (1, 1, 1, 1)
		_Smoothness("Smoothness", Vector) = (0.5, 0.5, 0.5, 0.5)

		_NHxRoughness("NHxRoughness", 2D) = "black" {}
	}

	SubShader
	{
		Pass
		{
			Tags { "LightMode" = "ForwardBase" }

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			#pragma multi_compile_fog

			#include "UnityCG.cginc"
			#include "Lighting.cginc"

			struct appdata
			{
				half4 vertex : POSITION;
				half2 uv : TEXCOORD0;
				half2 uv2 : TEXCOORD1;
				half3 normal : NORMAL;
				half4 tangent : TANGENT;
			};

			struct v2f
			{
				half4 pos : SV_POSITION;
				half4 pack0 : TEXCOORD0; // _Control _Splat0
				half4 pack1 : TEXCOORD1; // _Splat1 _Splat2
				half3 tspace0 : TEXCOORD2;
				half3 tspace1 : TEXCOORD3;
				half3 tspace2 : TEXCOORD4;
				float3 posWorld : TEXCOORD5;
				UNITY_FOG_COORDS(6)
				half2 uv2 : TEXCOORD7;
			};

			sampler2D _NHxRoughness;

			fixed4 _Color;
			sampler2D _Control;
			sampler2D _Splat0, _Splat1, _Splat2;
			sampler2D _Normal0, _Normal1, _Normal2;
			half4 _Splat0_ST;
			half4 _Splat1_ST;
			half4 _Splat2_ST;
			half4 _Control_ST;

			fixed3 _Spec0;
			fixed3 _Spec1;
			fixed3 _Spec2;
			half4 _Smoothness;

			v2f vert (appdata v)
			{
				v2f o;
				UNITY_INITIALIZE_OUTPUT(v2f, o);
				o.pos = UnityObjectToClipPos(v.vertex);
				
				half3 normal = UnityObjectToWorldNormal(v.normal);
                half3 tangent = UnityObjectToWorldDir(v.tangent.xyz);
				half tangentSign = v.tangent.w * unity_WorldTransformParams.w;
				half3 bitangent = cross(normal, tangent) * tangentSign;

				o.tspace0 = half3(tangent.x, bitangent.x, normal.x);
                o.tspace1 = half3(tangent.y, bitangent.y, normal.y);
                o.tspace2 = half3(tangent.z, bitangent.z, normal.z);
				o.posWorld = mul(unity_ObjectToWorld, v.vertex).xyz;

				o.pack0.xy = v.uv * _Control_ST.xy + _Control_ST.zw;
				o.pack0.zw = v.uv * _Splat0_ST.xy + _Splat0_ST.zw;
				o.pack1.xy = v.uv * _Splat1_ST.xy + _Splat1_ST.zw;
				o.pack1.zw = v.uv * _Splat2_ST.xy + _Splat2_ST.zw;

				o.uv2 = v.uv2.xy * unity_LightmapST.xy + unity_LightmapST.zw;

				UNITY_TRANSFER_FOG(o, o.pos);
				return o;
			}

			//inline half2 Pow4 (half2 x) { return x*x*x*x; }

			fixed4 frag (v2f i) : SV_Target
			{
				half2 uv_Control = i.pack0.xy;
				half2 uv_Splat0 = i.pack0.zw;
				half2 uv_Splat1 = i.pack1.xy;
				half2 uv_Splat2 = i.pack1.zw;

				fixed4 c = 0;
				fixed4 splat_control = tex2D(_Control, uv_Control);
				fixed4 lay0 = tex2D(_Splat0, uv_Splat0);
				fixed4 lay1 = tex2D(_Splat1, uv_Splat1);
				fixed4 lay2 = tex2D(_Splat2, uv_Splat2);
				fixed3 n0 = UnpackNormal(tex2D(_Normal0, uv_Splat0));
				fixed3 n1 = UnpackNormal(tex2D(_Normal1, uv_Splat1));
				fixed3 n2 = UnpackNormal(tex2D(_Normal2, uv_Splat2));

				half3 normal0;
                normal0.x = dot(i.tspace0, n0);
                normal0.y = dot(i.tspace1, n0);
                normal0.z = dot(i.tspace2, n0);
				normal0 = normalize(normal0);

				half3 normal1;
                normal1.x = dot(i.tspace0, n1);
                normal1.y = dot(i.tspace1, n1);
                normal1.z = dot(i.tspace2, n1);
				normal1 = normalize(normal1);

				half3 normal2;
                normal2.x = dot(i.tspace0, n2);
                normal2.y = dot(i.tspace1, n2);
                normal2.z = dot(i.tspace2, n2);
				normal2 = normalize(normal2);

				c.a = 1.0;
				c.rgb = (lay0.rgb * splat_control.r + lay1.rgb * splat_control.g + lay2.rgb * splat_control.b + lay0.rgb * splat_control.a) * _Color.rgb;
				
				half3 normal;
				normal = normalize(normal0 * splat_control.r + normal1 * splat_control.g + normal2 * splat_control.b + normal0 * splat_control.a);

				fixed3 _Spec;
				_Spec = (lay0.a * _Spec0 * splat_control.r + lay1.a * _Spec1 * splat_control.g + lay2.a * _Spec2 * splat_control.b + lay0.a * _Spec0 * splat_control.a);

				fixed Smoothness;
				Smoothness = (_Smoothness.x * splat_control.r + _Smoothness.y * splat_control.g + _Smoothness.z * splat_control.b + _Smoothness.x * splat_control.a);

				half3 viewDir = normalize(UnityWorldSpaceViewDir(i.posWorld));
				half3 lightDir = normalize(_WorldSpaceLightPos0.xyz);
				fixed3 lightColor = _LightColor0;

				fixed3 specColor = _Spec;
				half smoothness = Smoothness;

				half3 reflDir = reflect(viewDir, normal);
				half nl = saturate(dot(normal, lightDir));
				half nv = saturate(dot(normal, viewDir));
				half2 rlPow4AndFresnelTerm = Pow4(half2(dot(reflDir, lightDir), 1 - nv));
				half rlPow4 = rlPow4AndFresnelTerm.x;
				half LUT_RANGE = 16.0;
				half specular = tex2D(_NHxRoughness, half2(rlPow4, 1 - smoothness)).UNITY_ATTEN_CHANNEL * LUT_RANGE;

				fixed3 lm = DecodeLightmap(UNITY_SAMPLE_TEX2D(unity_Lightmap, i.uv2));

				fixed3 diffuse = c.rgb * lm + lightColor * nl * c.rgb;
				fixed3 spec = lightColor * specular * specColor;

				c.rgb = diffuse + spec;

				// apply fog
				UNITY_APPLY_FOG(i.fogCoord, c);
				return c;
			}
			ENDCG
		}
	}

	Fallback "YuLongZhi/ShadowTerrain"
}