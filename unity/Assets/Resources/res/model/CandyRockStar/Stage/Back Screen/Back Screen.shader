// Upgrade NOTE: replaced 'mul(UNITY_MATRIX_MVP,*)' with 'UnityObjectToClipPos(*)'

Shader "UnityChan/Back Screen"
{
    Properties
    {
        _MainTex      ("Base",          2D) = ""{}
        _StripeTex    ("Stripe",        2D) = ""{}
        _Params ("Params", Vector) = (1, 1, 1, 1)
    }

    CGINCLUDE

    #include "UnityCG.cginc"

    struct v2f
    {
        float4 position : SV_POSITION;
        float2 uv0 : TEXCOORD0;
        float2 uv1 : TEXCOORD1;
    };

    sampler2D _MainTex;
    float4 _MainTex_ST;

    sampler2D _StripeTex;
    float4 _StripeTex_ST;
    
    float4 _Params; // _BaseLevel _StripeLevel _FlickerLevel _FlickerFreq

    v2f vert(appdata_base v)
    {
        v2f o;
        o.position = UnityObjectToClipPos(v.vertex);
        o.uv0 = TRANSFORM_TEX(v.texcoord, _MainTex);
        o.uv1 = TRANSFORM_TEX(v.texcoord, _StripeTex);
        return o;
    }

    float4 frag(v2f i) : COLOR
    {
        float4 color = tex2D(_MainTex, i.uv0);

        float amp = tex2D(_StripeTex, i.uv1).r;
        amp = _Params.x + _Params.y * amp;

        float time = _Time.y * 3.14f * _Params.w;
        float flicker = lerp(1.0f, sin(time) * 0.5f, _Params.z);

        return color * (amp * flicker);
    }

    ENDCG

    SubShader
    {
        Tags { "RenderType"="Opaque" }
        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            ENDCG
        }
    } 
    FallBack "Diffuse"
}
