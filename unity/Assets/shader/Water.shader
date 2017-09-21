Shader "Water" {
    Properties {
		_MainTex ("MainTex", 2D) = "white" {}
		_Color ("Color", Color) = (0.5, 0.5, 0.5, 1)
		_Speed ("Speed", Float) = 1
        _Distort ("Distort", Float) = 1
        _Intensity ("Intensity", Float ) = 1.5
		_Alpha ("Alpha", Range(0, 5) ) = 3
    }

    SubShader {
		Tags { "Queue" = "Transparent" }
		Cull Off
		ZWrite Off
		Blend SrcAlpha OneMinusSrcAlpha
		Offset -1, -1

        Pass {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
			#pragma multi_compile_fog
			#include "UnityCG.cginc"

			uniform sampler2D _MainTex;
			uniform float4 _MainTex_ST;
			uniform float _Speed;
			uniform float4 _Color;
            uniform float _Distort;
            uniform float _Alpha;
            uniform float _Intensity;

            struct VertexInput {
                float4 vertex : POSITION;
                float2 texcoord0 : TEXCOORD0;
            };

            struct VertexOutput {
                float4 pos : SV_POSITION;
                float2 uv0 : TEXCOORD0;
                UNITY_FOG_COORDS(1)
            };

            VertexOutput vert (VertexInput v) {
                VertexOutput o = (VertexOutput)0;
                o.uv0 = v.texcoord0;
                o.pos = UnityObjectToClipPos(v.vertex );
                UNITY_TRANSFER_FOG(o,o.pos);
                return o;
            }

            float4 frag(VertexOutput i) : COLOR {
                float time = _Time.y * _Speed;

                float2 st_0 = (i.uv0 + time * float2(0, 0.05));
                float4 color_0 = tex2D(_MainTex, TRANSFORM_TEX(st_0, _MainTex));

                float2 st_1 = (i.uv0 + time * float2(-0.01, 0.05));
                float4 color_1 = tex2D(_MainTex, TRANSFORM_TEX(st_1, _MainTex));

                float2 st_2 = i.uv0 + time * float2(0.03, 0.03) + color_0.rg * color_1.rg * _Distort;
                float4 color_2 = tex2D(_MainTex, TRANSFORM_TEX(st_2, _MainTex));

				float3 color_3 = _Color.rgb * color_2.rgb * _Intensity;

				fixed4 final = fixed4(color_3, (color_2.b * _Alpha));

                UNITY_APPLY_FOG(i.fogCoord, final);
                return final;
            }
            ENDCG
        }
    }
}