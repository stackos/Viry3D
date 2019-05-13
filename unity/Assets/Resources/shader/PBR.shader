Shader "PBR"
{
	Properties
	{
        u_color ("Color", Color) = (1, 1, 1, 1)
        [NoScaleOffset] u_texture ("MainTex", 2D) = "white" {}
        [NoScaleOffset] u_normal ("Normal", 2D) = "bump" {}
        [NoScaleOffset] u_metallic_smoothness ("MetallicSmoothness (RG)", 2D) = "white" {}
        u_metallic ("Metallic", Range(0, 1)) = 0.0
        u_smoothness ("Smoothness", Range(0, 1)) = 0.5
        [NoScaleOffset] u_occlusion ("Occlusion", 2D) = "white" {}
        u_occlusion_strength ("OcclusionStrength", Range(0, 1)) = 1.0
        [NoScaleOffset] u_emission ("Emission", 2D) = "white" {}
        u_emission_color ("EmissionColor", Color) = (0, 0, 0, 1)

        [NoScaleOffset] u_environment("Environment", Cube) = "" {}
        u_ambient("Ambient", Color) = (0, 0, 0, 1)
        u_lightDir("LightDir", Vector) = (0, 0, 1, 0)
        u_lightColor("LightColor", Color) = (1, 1, 1, 1)
	}
	SubShader
	{
		Pass
		{
            Tags { "LightMode" = "ForwardBase" }

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
            #define surface_input surf_in
            #define surface surf

			#include "UnityCG.cginc"

            struct Input
            {
                float2 uv;
            };

            half3 u_color;
            sampler2D u_texture;
            sampler2D u_normal;
            sampler2D u_metallic_smoothness;
            half u_metallic;
            half u_smoothness;
            sampler2D u_occlusion;
            half u_occlusion_strength;
            sampler2D u_emission;
            half3 u_emission_color;

            #include "PBR.cginc"

            void surf_in(v2f i, inout Input IN)
            {
                IN.uv = i.uv.xy;
            }

            void surf(Input IN, inout SurfaceOutputStandard o)
            {
                fixed4 albedo = tex2D(u_texture, IN.uv);
                float3 normal = UnpackNormal(tex2D(u_normal, IN.uv));
                half4 metallicSmoothness = tex2D(u_metallic_smoothness, IN.uv);
                half metallic = metallicSmoothness.r * u_metallic;
                half smoothness = metallicSmoothness.g * u_smoothness;
                half occlusion = lerp(1, tex2D(u_occlusion, IN.uv).g, u_occlusion_strength);
                half3 emission = tex2D(u_emission, IN.uv) * u_emission_color;

                o.Albedo = albedo.rgb * u_color;
                o.Normal = normal;
                o.Metallic = metallic;
                o.Smoothness = smoothness;
                o.Occlusion = occlusion;
                o.Emission = emission;
                o.Alpha = albedo.a;
            }
			ENDCG
		}
	}
}
