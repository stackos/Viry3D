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

Shader "irradiance"
{
	Properties
	{
		_CubeMap ("_CubeMap", CUBE) = "" {}
	}

	SubShader
	{
		Cull Off

		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag

			#include "UnityCG.cginc"

			struct appdata
			{
				float4 vertex : POSITION;
			};

			struct v2f
			{
				float4 pos : SV_POSITION;
				float3 posWorld : TEXCOORD1;
			};

			samplerCUBE _CubeMap;

			v2f vert (appdata v)
			{
				v2f o;
				o.pos = UnityObjectToClipPos(v.vertex);
				o.posWorld = mul(unity_ObjectToWorld, v.vertex).xyz;

				return o;
			}

			float4 frag (v2f i) : SV_Target
			{
				float3 N = normalize(i.posWorld);

				float3 irradiance = float3(0.0, 0.0, 0.0);

				// tangent space calculation from origin point
				float3 up = float3(0.0, 1.0, 0.0);
				float3 right = cross(up, N);
				up = cross(N, right);

				const float PI = 3.14159265359;

				float sampleDelta = 0.025;
				float nrSamples = 0.0f;
				for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
				{
					for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
					{
						// spherical to cartesian (in tangent space)
						float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
						// tangent space to world
						float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

						irradiance += pow(texCUBE(_CubeMap, sampleVec).rgb, 2.2) * cos(theta) * sin(theta);
						nrSamples++;
					}
				}
				irradiance = PI * irradiance * (1.0 / float(nrSamples));

				return float4(irradiance, 1.0);
			}
			ENDCG
		}
	}
}
