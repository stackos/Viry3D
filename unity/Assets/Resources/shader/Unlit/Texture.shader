Shader "Unlit/Texture"
{
	Properties
	{
		[NoScaleOffset] u_texture ("Texture", 2D) = "white" { }
		u_texture_scale_offset ("TextureScaleOffset", Vector) = (1, 1, 0, 0)
	}
	SubShader
	{
		Pass
		{
			Tags { "LightMode" = "ForwardBase" }

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
				float2 uv : TEXCOORD0;
				float4 vertex : SV_POSITION;
			};

			sampler2D u_texture;
			float4 u_texture_scale_offset;

			v2f vert (appdata v)
			{
				v2f o;
				o.vertex = UnityObjectToClipPos(v.vertex);
				o.uv = v.uv * u_texture_scale_offset.xy + u_texture_scale_offset.zw;
				return o;
			}
			
			fixed4 frag (v2f i) : SV_Target
			{
				fixed4 c = tex2D(u_texture, i.uv);
				return c;
			}
			ENDCG
		}
	}
}
