Shader "Dissolve" {

Properties {
    _MainTex ("MainTex", 2D) = "white" {}

    _DissolveTex ("DissolveTex", 2D) = "white" {}
    _DissolveAmount ("DissolveAmount", Range (0, 1)) = 0.5
    _DissBorderSize ("DissBorderSize", Range (0, 1)) = 0.1
    _DissBorderColor1 ("DissBorderColor1", Color) = (0, 0.9, 1, 1)
    _DissBorderColor2 ("DissBorderColor2", Color) = (0, 0.5, 1, 1)
}

SubShader {
    Pass {
        CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
 
            #include "UnityCG.cginc"
      
            sampler2D _MainTex;

			sampler2D _DissolveTex;
            half _DissolveAmount, _DissBorderSize;
            half4 _DissBorderColor1, _DissBorderColor2; 
            
            struct appdata_t {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;    
            };
      
            struct v2f {
                float4 vertex : SV_POSITION;
                half2 uv : TEXCOORD0;
            };
                  
            v2f vert (appdata_t v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = v.uv;
                return o;
            }

			float DissolveClip(half2 uv)
			{
				float dissolve = tex2D(_DissolveTex, uv).r;
                float clipAmount = dissolve - _DissolveAmount - 0.001;
                clip(clipAmount);

				return clipAmount;
			}

			fixed4 DissolveBorder(fixed4 color, float clipAmount)
			{
				if(clipAmount < _DissBorderSize)
                {
                    color = lerp(_DissBorderColor1, _DissBorderColor2, clipAmount / _DissBorderSize);
                }

				return color;
			}
                  
            fixed4 frag (v2f i) : SV_Target
            {
				float clipAmount = DissolveClip(i.uv);

                fixed4 c = tex2D(_MainTex, i.uv);

                c = DissolveBorder(c, clipAmount);

                return c;
            }
        ENDCG
    }
}

}  