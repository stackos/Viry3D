UniformPush(push_constant) uniform push {
	mat4 _World;
} u_push;

UniformBuffer(std140, binding = 1) uniform buf_vs {
	mat4 _ViewProjection;
	vec4 _Bones[BONE_VEC_MAX];
} u_buf;

layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_blend_weight;
layout (location = 3) in vec4 a_blend_indices;

Varying(location = 0) out vec2 v_uv;

void main() {
    vec4 skinned_pos_world;
    skinned_mesh(skinned_pos_world, a_pos, a_blend_weight, a_blend_indices, u_buf._Bones);
    gl_Position = skinned_pos_world * u_buf._ViewProjection;
	v_uv = a_uv;

	vulkan_convert();
}