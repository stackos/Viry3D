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

UniformBuffer(0, 2) uniform buf_vs {
	mat4 _ViewProjection;
	vec4 _TintColor;
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

Varying(0) out vec2 v_uv;
Varying(1) out vec4 v_color;

void main() {
	gl_Position = a_pos * u_buf._ViewProjection;
	v_uv = a_uv;
	v_color = a_color * u_buf._TintColor * 2.0;

	vulkan_convert();
}
