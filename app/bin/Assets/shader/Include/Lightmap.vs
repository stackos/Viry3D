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
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec2 a_uv2;

Varying(0) out vec2 v_uv;
Varying(1) out vec2 v_uv2;

void main() {
	vec4 pos = a_pos * u_buf_obj._World;
	gl_Position = pos * u_buf._ViewProjection;
	v_uv = a_uv;
	v_uv2 = vec2(a_uv2.x, 1.0 - a_uv2.y) * u_buf_obj._LightmapScaleOffset.xy + u_buf_obj._LightmapScaleOffset.zw;
	v_uv2.y = 1.0 - v_uv2.y;
	
	vulkan_convert();
}