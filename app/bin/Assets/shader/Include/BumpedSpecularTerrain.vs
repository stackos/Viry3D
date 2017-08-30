UniformPush(push_constant) uniform push {
	mat4 _World;
	vec4 _LightmapScaleOffset;
} u_push;

UniformBuffer(std140, binding = 1) uniform buf_vs {
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

Varying(location = 0) out vec4 v_pack0;
Varying(location = 1) out vec4 v_pack1;
Varying(location = 2) out vec4 v_t2w0;
Varying(location = 3) out vec4 v_t2w1;
Varying(location = 4) out vec4 v_t2w2;
Varying(location = 5) out vec2 v_uv2;

void main() {
	vec4 pos = a_pos * u_push._World;
	gl_Position = pos * u_buf._ViewProjection;
	v_uv2 = vec2(a_uv2.x, 1.0 - a_uv2.y) * u_push._LightmapScaleOffset.xy + u_push._LightmapScaleOffset.zw;
	v_uv2.y = 1.0 - v_uv2.y;
	
	vec3 normal = (vec4(a_normal, 0.0) * u_push._World).xyz;
	vec3 tangent = (vec4(a_tangent.xyz, 0.0) * u_push._World).xyz;
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