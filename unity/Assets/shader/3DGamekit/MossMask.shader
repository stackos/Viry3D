Shader "3DGamekit/MossMask"
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
		CGPROGRAM
		#pragma surface surf Standard
		#pragma target 3.0

		sampler2D _MainTex;
        sampler2D _Normal;
        sampler2D _DetailNormal;
        sampler2D _Occlusion;
        sampler2D _MetallicSmoothness;

        half _DetailScale;
        half _OcclusionStrength;
        half _Metallic;
        half _Smoothness;

		struct Input
        {
			float2 uv_MainTex;
		};

        half3 blend_rnm(half3 n1, half3 n2)
        {
            n1.z += 1;
            n2.xy = -n2.xy;

            return n1 * dot(n1, n2) / n1.z - n2;
        }

		void surf (Input IN, inout SurfaceOutputStandard o)
        {
			fixed4 color = tex2D(_MainTex, IN.uv_MainTex);

            half3 normal = UnpackNormal(tex2D(_Normal, IN.uv_MainTex));
            half3 detailNormal = UnpackNormal(tex2D(_DetailNormal, IN.uv_MainTex * _DetailScale));
            half occlusion = lerp(1, tex2D(_Occlusion, IN.uv_MainTex), _OcclusionStrength);
            half4 metallicSmoothness = tex2D(_MetallicSmoothness, IN.uv_MainTex);

            normal = blend_rnm(normal, detailNormal);
            half metallic = metallicSmoothness.r * _Metallic;
            half smoothness = metallicSmoothness.a * _Smoothness;

			o.Albedo = color.rgb;
            o.Normal = normal;
            o.Occlusion = occlusion;
			o.Metallic = metallic;
			o.Smoothness = smoothness;
			o.Alpha = color.a;
		}
		ENDCG
	}
}
