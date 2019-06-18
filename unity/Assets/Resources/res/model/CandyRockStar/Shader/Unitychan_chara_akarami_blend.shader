Shader "UnityChan/Blush - Transparent"
{
	Properties
	{
		_ShadowColor ("Shadow Color", Color) = (0.8, 0.8, 1, 1)

		_FalloffSampler ("Falloff Control", 2D) = "white" {}
		_RimLightSampler ("RimLight Control", 2D) = "white" {}

		[NoScaleOffset] u_texture ("Texture", 2D) = "white" { }
		u_texture_scale_offset ("TextureScaleOffset", Vector) = (1, 1, 0, 0)
		u_color ("Main Color", Color) = (1, 1, 1, 1)
	}

	SubShader
	{
		Blend SrcAlpha OneMinusSrcAlpha, One One 
		ZWrite Off
		Tags
		{
			"Queue"="Geometry+3"
			"IgnoreProjector"="True"
			"RenderType"="Overlay"
			"LightMode"="ForwardBase"
		}
		
		Pass
		{
			Cull Back
			ZTest LEqual
CGPROGRAM
#pragma multi_compile_fwdbase
#pragma vertex vert
#pragma fragment frag
#include "UnityCG.cginc"
#include "AutoLight.cginc"
#define ENABLE_CAST_SHADOWS
#define ENABLE_RIMLIGHT
#include "CharaSkin.cg"
ENDCG
		}
	}
}

