Shader "PBR"
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
		irradianceMap("irradianceMap", CUBE) = "white" { }
		prefilterMap("prefilterMap", CUBE) = "white" { }
		_LightDir("LightDir", Vector) = (0, 0, 1, 0)
		_LightColor("LightColor", Color) = (1, 1, 1, 1)
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
				half3 tspace0 : TEXCOORD1;
				half3 tspace1 : TEXCOORD2;
				half3 tspace2 : TEXCOORD3;
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
			half3 _LightDir;
			fixed4 _LightColor;

			v2f vert (appdata v)
			{
				v2f o;
				o.pos = UnityObjectToClipPos(v.vertex);
				o.uv = v.uv;

				o.posWorld = mul(unity_ObjectToWorld, v.vertex).xyz;

				half3 normal = UnityObjectToWorldNormal(v.normal);
				half3 tangent = UnityObjectToWorldDir(v.tangent.xyz);
				half tangentSign = v.tangent.w * unity_WorldTransformParams.w;
				half3 bitangent = cross(normal, tangent) * tangentSign;

				o.tspace0 = half3(tangent.x, bitangent.x, normal.x);
				o.tspace1 = half3(tangent.y, bitangent.y, normal.y);
				o.tspace2 = half3(tangent.z, bitangent.z, normal.z);

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

			fixed4 frag (v2f i) : SV_Target
			{
				fixed3 n = UnpackNormal(tex2D(normalMap, i.uv));

				half3 normal;
				normal.x = dot(i.tspace0, n);
				normal.y = dot(i.tspace1, n);
				normal.z = dot(i.tspace2, n);
				normal = normalize(normal);

				half3 viewDir = normalize(UnityWorldSpaceViewDir(i.posWorld));

				half3 lightDir = normalize(-_LightDir);
				float3 lightColor = _LightColor;

				float3 albedo = pow(tex2D(albedoMap, i.uv).rgb * _albedo.rgb, float3(2.2, 2.2, 2.2));
				float metallic = tex2D(metallicMap, i.uv).r * _metallic;
				float roughness = tex2D(roughnessMap, i.uv).r * _roughness;
				float ao = tex2D(aoMap, i.uv).r * _ao;

				float3 N = normal;
				float3 V = viewDir;
				float3 R = reflect(-V, N);

				float3 F0 = float3(0.04, 0.04, 0.04);
				F0 = lerp(F0, albedo, metallic);

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
					Lo = (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
				}

				float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

				float3 kS = F;
				float3 kD = 1.0 - kS;
				kD *= 1.0 - metallic;

				float3 irradiance = texCUBE(irradianceMap, N).rgb;
				float3 diffuse = irradiance * albedo;

				const float MAX_REFLECTION_LOD = 4.0;
				float3 prefilteredColor = texCUBElod(prefilterMap, float4(R, roughness * MAX_REFLECTION_LOD)).rgb;
				float2 brdf = tex2D(brdfLUT, float2(max(dot(N, V), 0.0), 1 - roughness)).rg;
				float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

				float3 ambient = (kD * diffuse + specular) * ao;

				float3 color = ambient + Lo;

				color = color / (color + float3(1.0, 1.0, 1.0));
				// gamma correct
				color = pow(color, 1.0 / 2.2);

				return float4(color, 1.0);
			}
			ENDCG
		}
	}
}