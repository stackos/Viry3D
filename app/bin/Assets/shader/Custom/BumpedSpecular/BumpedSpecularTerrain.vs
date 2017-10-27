/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

UniformBuffer(1, 0) uniform buf_vs_obj {
	mat4 _World;
	vec4 _LightmapScaleOffset;
} u_buf_obj;

UniformBuffer(0, 2) uniform buf_vs {
	mat4 _ViewProjection;
	vec4 _Splat0_ST;
	vec4 _Splat1_ST;
	vec4 _Splat2_ST;
	vec4 _Control_ST;
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec2 a_uv2;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in vec4 a_tangent;

Varying(0) out vec4 v_pack0;
Varying(1) out vec4 v_pack1;
Varying(2) out vec4 v_t2w0;
Varying(3) out vec4 v_t2w1;
Varying(4) out vec4 v_t2w2;
Varying(5) out vec2 v_uv2;

void main() {
	vec4 pos = a_pos * u_buf_obj._World;
	gl_Position = pos * u_buf._ViewProjection;
	v_uv2 = vec2(a_uv2.x, 1.0 - a_uv2.y) * u_buf_obj._LightmapScaleOffset.xy + u_buf_obj._LightmapScaleOffset.zw;
	v_uv2.y = 1.0 - v_uv2.y;
	
	vec3 normal = (vec4(a_normal, 0.0) * u_buf_obj._World).xyz;
	vec3 tangent = (vec4(a_tangent.xyz, 0.0) * u_buf_obj._World).xyz;
	vec3 binormal = cross(normal, tangent) * a_tangent.w;

	v_t2w0 = vec4(tangent.x, binormal.x, normal.x, pos.x);
	v_t2w1 = vec4(tangent.y, binormal.y, normal.y, pos.y);
	v_t2w2 = vec4(tangent.z, binormal.z, normal.z, pos.z);

	v_pack0.xy = a_uv * u_buf._Control_ST.xy + u_buf._Control_ST.zw;
	v_pack0.zw = a_uv * u_buf._Splat0_ST.xy + u_buf._Splat0_ST.zw;
	v_pack1.xy = a_uv * u_buf._Splat1_ST.xy + u_buf._Splat1_ST.zw;
	v_pack1.zw = a_uv * u_buf._Splat2_ST.xy + u_buf._Splat2_ST.zw;
	
	vulkan_convert();
}
