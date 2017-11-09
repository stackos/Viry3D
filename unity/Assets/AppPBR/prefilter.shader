Shader "prefilter"
{
	Properties
	{
		_MainTex("MainTex", 2D) = "white" { }
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
			};

			struct v2f
			{
				float4 pos : SV_POSITION;
				float2 uv : TEXCOORD0;
			};

			sampler2D _MainTex;

			v2f vert (appdata v)
			{
				v2f o;
				o.pos = UnityObjectToClipPos(v.vertex);
				o.uv = v.uv;

				return o;
			}

			float4 frag (v2f i) : SV_Target
			{
				float4 c = pow(tex2D(_MainTex, i.uv), 2.2);
				return c;
			}
			ENDCG
		}
	}
}
