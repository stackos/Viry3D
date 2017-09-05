UniformBuffer(0, 1) uniform buf_vs {
	mat4 _ViewProjection;
} u_buf;

UniformBuffer(1, 0) uniform buf_vs_obj {
	mat4 _World;
} u_buf_obj;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

Varying(0) out vec2 v_uv;
Varying(1) out vec4 v_color;

void main() {
	gl_Position = a_pos * u_buf_obj._World * u_buf._ViewProjection;
	v_uv = a_uv;
	v_color = a_color;

	vulkan_convert();
}