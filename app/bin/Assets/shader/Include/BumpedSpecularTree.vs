UniformPush(push_constant) uniform push {
	mat4 _World;
	vec4 _LightmapScaleOffset;
} u_push;

UniformBuffer(std140, binding = 1) uniform buf_vs {
	mat4 _ViewProjection;
	vec4 _Time;
	vec4 _Frequency;
	vec4 _Wind;
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec2 a_uv2;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in vec4 a_tangent;
layout (location = 5) in vec4 a_color;

Varying(location = 0) out vec2 v_uv;
Varying(location = 1) out vec2 v_uv2;
Varying(location = 2) out vec4 v_t2w0;
Varying(location = 3) out vec4 v_t2w1;
Varying(location = 4) out vec4 v_t2w2;

void main() {
	float wave_offset = a_color.r;
	float wave_weight = a_color.g;

	vec4 pos;
	pos.xyz = a_pos.xyz + u_buf._Wind.xyz * cos(u_buf._Time.x * u_buf._Frequency.x + wave_offset * 3.14159) * wave_weight;
	pos.w = 1.0;
	
	pos = pos * u_push._World;
	gl_Position = pos * u_buf._ViewProjection;
	v_uv = a_uv;
	v_uv2 = vec2(a_uv2.x, 1.0 - a_uv2.y) * u_push._LightmapScaleOffset.xy + u_push._LightmapScaleOffset.zw;
	v_uv2.y = 1.0 - v_uv2.y;
	
	vec3 normal = (vec4(a_normal, 0.0) * u_push._World).xyz;
	vec3 tangent = (vec4(a_tangent.xyz, 0.0) * u_push._World).xyz;
	vec3 binormal = cross(normal, tangent) * a_tangent.w;

	v_t2w0 = vec4(tangent.x, binormal.x, normal.x, pos.x);
	v_t2w1 = vec4(tangent.y, binormal.y, normal.y, pos.y);
	v_t2w2 = vec4(tangent.z, binormal.z, normal.z, pos.z);
	
	vulkan_convert();
}