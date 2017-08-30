UniformPush(push_constant) uniform push {
	mat4 _World;
} u_push;

UniformBuffer(std140, binding = 1) uniform buf_vs {
	mat4 _ViewProjection;
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

Varying(location = 0) out vec2 v_uv;
Varying(location = 1) out vec4 v_color;

void main() {
	gl_Position = a_pos * u_push._World * u_buf._ViewProjection;
	v_uv = a_uv;
	v_color = a_color;

	vulkan_convert();
}