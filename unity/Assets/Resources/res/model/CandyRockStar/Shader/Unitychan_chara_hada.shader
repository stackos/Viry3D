Shader "UnityChan/Skin"
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

CGINCLUDE
#include "UnityCG.cginc"
#include "AutoLight.cginc"
ENDCG

	SubShader
	{
		Tags
		{
			"RenderType"="Opaque"
			"Queue"="Geometry"
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
#define ENABLE_CAST_SHADOWS
#define ENABLE_RIMLIGHT
#include "CharaSkin.cg"
ENDCG
		}
	}
}

