/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

Shader "PBR1"
{
	Properties
	{
		_albedo("albedo", Color) = (1, 1, 1, 1)
		albedoMap("albedoMap", 2D) = "white" { }
		normalMap("normalMap", 2D) = "bump" { }
		metallicMap("metallicMap", 2D) = "white" { }
		roughnessMap("roughnessMap", 2D) = "white" { }
		aoMap("aoMap", 2D) = "white" { }
		brdfLUT("brdfLUT", 2D) = "white" { }
		_metallic("metallic", Range(0, 1)) = 1
		_roughness("roughness", Range(0, 1)) = 1
		_ao("ao", Range(0, 1)) = 1
		irradianceMap("irradianceMap", CUBE) = "" { }
		prefilterMap("prefilterMap", CUBE) = "" { }
		_LightDir("LightDir", Vector) = (0, 0, 1, 0)
		_LightColor("LightColor", Color) = (1, 1, 1, 1)
		_LightIntensity("LightIntensity", Range(0, 10)) = 1
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
				float3 normal : NORMAL;
				float4 tangent : TANGENT;
			};

			struct v2f
			{
				float4 pos : SV_POSITION;
				float2 uv : TEXCOORD0;
				float3 tspace0 : TEXCOORD1;
				float3 tspace1 : TEXCOORD2;
				float3 tspace2 : TEXCOORD3;
				float3 posWorld : TEXCOORD4;
			};

			sampler2D albedoMap;
			float4 _albedo;
			sampler2D normalMap;
			sampler2D metallicMap;
			float _metallic;
			sampler2D roughnessMap;
			float _roughness;
			sampler2D aoMap;
			float _ao;
			sampler2D brdfLUT;
			samplerCUBE irradianceMap;
			samplerCUBE prefilterMap;
			float3 _LightDir;
			float4 _LightColor;
			float _LightIntensity;

			v2f vert (appdata v)
			{
				v2f o;
				o.pos = UnityObjectToClipPos(v.vertex);
				o.uv = v.uv;

				o.posWorld = mul(unity_ObjectToWorld, v.vertex).xyz;

				float3 normal = UnityObjectToWorldNormal(v.normal);
				float3 tangent = UnityObjectToWorldDir(v.tangent.xyz);
				float tangentSign = v.tangent.w * unity_WorldTransformParams.w;
				float3 bitangent = cross(normal, tangent) * tangentSign;

				o.tspace0 = float3(tangent.x, bitangent.x, normal.x);
				o.tspace1 = float3(tangent.y, bitangent.y, normal.y);
				o.tspace2 = float3(tangent.z, bitangent.z, normal.z);

				return o;
			}
			
			float DistributionGGX(float3 N, float3 H, float roughness)
			{
				float a = roughness*roughness;
				float a2 = a*a;
				float NdotH = max(dot(N, H), 0.0);
				float NdotH2 = NdotH*NdotH;

				const float PI = 3.14159265359;
				float nom = a2;
				float denom = (NdotH2 * (a2 - 1.0) + 1.0);
				denom = PI * denom * denom;

				return nom / denom;
			}

			float GeometrySchlickGGX(float NdotV, float roughness)
			{
				float r = (roughness + 1.0);
				float k = (r*r) / 8.0;

				float nom = NdotV;
				float denom = NdotV * (1.0 - k) + k;

				return nom / denom;
			}

			float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
			{
				float NdotV = max(dot(N, V), 0.0);
				float NdotL = max(dot(N, L), 0.0);
				float ggx2 = GeometrySchlickGGX(NdotV, roughness);
				float ggx1 = GeometrySchlickGGX(NdotL, roughness);

				return ggx1 * ggx2;
			}

			float3 fresnelSchlick(float cosTheta, float3 F0)
			{
				return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
			}

			float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
			{
				return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
			}

			float4 frag (v2f i) : SV_Target
			{
				float3 n = UnpackNormal(tex2D(normalMap, i.uv));

				float3 normal;
				normal.x = dot(i.tspace0, n);
				normal.y = dot(i.tspace1, n);
				normal.z = dot(i.tspace2, n);
				normal = normalize(normal);

				float3 viewDir = normalize(UnityWorldSpaceViewDir(i.posWorld));
				float3 lightDir = normalize(-_LightDir);
				float3 lightColor = _LightColor * _LightIntensity;

				// material properties
				float3 albedo = pow(tex2D(albedoMap, i.uv).rgb * _albedo.rgb, 2.2);
				//float3 albedo = tex2D(albedoMap, i.uv).rgb * _albedo.rgb;
				float metallic = tex2D(metallicMap, i.uv).r * _metallic;
				float roughness = tex2D(roughnessMap, i.uv).r * _roughness;
				float ao = tex2D(aoMap, i.uv).r * _ao;

				// input lighting data
				float3 N = normal;
				float3 V = viewDir;
				float3 R = reflect(-V, N);

				// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
				// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)  
				float3 F0 = float3(0.04, 0.04, 0.04);
				F0 = lerp(F0, albedo, metallic);

				// reflectance equation
				float3 Lo = float3(0.0, 0.0, 0.0);
				{
					float3 L = lightDir;
					float3 H = normalize(V + L);
					float3 radiance = lightColor;

					// Cook-Torrance BRDF
					float NDF = DistributionGGX(N, H, roughness);
					float G = GeometrySmith(N, V, L, roughness);
					float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

					float3 nominator = NDF * G * F;
					float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
					float3 specular = nominator / denominator;

					// kS is equal to Fresnel
					float3 kS = F;
					// for energy conservation, the diffuse and specular light can't
					// be above 1.0 (unless the surface emits light); to preserve this
					// relationship the diffuse component (kD) should equal 1.0 - kS.
					float3 kD = float3(1.0, 1.0, 1.0) - kS;
					// multiply kD by the inverse metalness such that only non-metals
					// have diffuse lighting, or a linear blend if partly metal (pure metals
					// have no diffuse light).
					kD *= 1.0 - metallic;

					// scale light by NdotL
					float NdotL = max(dot(N, L), 0.0);

					// add to outgoing radiance Lo
					const float PI = 3.14159265359;
					Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
				}

				// ambient lighting (we now use IBL as the ambient term)
				float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

				float3 kS = F;
				float3 kD = 1.0 - kS;
				kD *= 1.0 - metallic;

				float3 irradiance = texCUBE(irradianceMap, N).rgb;
				float3 diffuse = irradiance * albedo;

				const float MAX_REFLECTION_LOD = 4.0;
				float3 prefilteredColor = texCUBElod(prefilterMap, float4(R, roughness * MAX_REFLECTION_LOD)).rgb;
				float2 brdf = tex2D(brdfLUT, float2(max(dot(N, V), 0.0), roughness)).rg;
				float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

				float3 ambient = (kD * diffuse + specular) * ao;

				float3 color = ambient + Lo;

				// HDR tonemapping
				color = color / (color + float3(1.0, 1.0, 1.0));
				// gamma correct
				color = pow(color, 1.0 / 2.2);

				return float4(color, 1.0);
			}
			ENDCG
		}
	}
}