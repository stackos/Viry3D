local vs = [[
#ifndef SKIN_ON
	#define SKIN_ON 0
#endif

VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
	vec4 u_camera_pos;
};
VK_UNIFORM_BINDING(1) uniform PerRenderer
{
	mat4 u_model_matrix;
};
VK_UNIFORM_BINDING(3) uniform PerMaterialVertex
{
	vec4 u_texture_scale_offset;
};
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
layout(location = 4) in vec3 i_normal;
VK_LAYOUT_LOCATION(0) out vec3 v_pos;
VK_LAYOUT_LOCATION(1) out vec2 v_uv;
VK_LAYOUT_LOCATION(2) out vec3 v_normal;
VK_LAYOUT_LOCATION(3) out vec3 v_camera_pos;

#if (SKIN_ON == 1)
	VK_UNIFORM_BINDING(2) uniform PerRendererBones
	{
		vec4 u_bones[210];
	};
	layout(location = 6) in vec4 i_bone_weights;
	layout(location = 7) in vec4 i_bone_indices;
	mat4 skin_mat()
	{
		int index_0 = int(i_bone_indices.x);
		int index_1 = int(i_bone_indices.y);
		int index_2 = int(i_bone_indices.z);
		int index_3 = int(i_bone_indices.w);
		float weights_0 = i_bone_weights.x;
		float weights_1 = i_bone_weights.y;
		float weights_2 = i_bone_weights.z;
		float weights_3 = i_bone_weights.w;
		mat4 bone_0 = mat4(u_bones[index_0*3], u_bones[index_0*3+1], u_bones[index_0*3+2], vec4(0, 0, 0, 1));
		mat4 bone_1 = mat4(u_bones[index_1*3], u_bones[index_1*3+1], u_bones[index_1*3+2], vec4(0, 0, 0, 1));
		mat4 bone_2 = mat4(u_bones[index_2*3], u_bones[index_2*3+1], u_bones[index_2*3+2], vec4(0, 0, 0, 1));
		mat4 bone_3 = mat4(u_bones[index_3*3], u_bones[index_3*3+1], u_bones[index_3*3+2], vec4(0, 0, 0, 1));
		return bone_0 * weights_0 + bone_1 * weights_1 + bone_2 * weights_2 + bone_3 * weights_3;
	}
#endif

void main()
{
#if (SKIN_ON == 1)
    mat4 model_matrix = skin_mat();
#else
    mat4 model_matrix = u_model_matrix;
#endif
	
	vec4 world_pos = i_vertex * model_matrix;
	gl_Position = world_pos * u_view_matrix * u_projection_matrix;
	v_pos = world_pos.xyz;
	v_uv = i_uv * u_texture_scale_offset.xy + u_texture_scale_offset.zw;
    v_normal = (vec4(i_normal, 0.0) * model_matrix).xyz;
	v_camera_pos = u_camera_pos.xyz;

	vk_convert();
}
]]

local fs = [[
#ifndef LIGHT_ADD_ON
	#define LIGHT_ADD_ON 0
#endif

#define FALLOFF_POWER 1.0
precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_SAMPLER_BINDING(1) uniform sampler2D _FalloffSampler;
VK_SAMPLER_BINDING(2) uniform sampler2D _RimLightSampler;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 u_color;
	vec4 _ShadowColor;
};
VK_UNIFORM_BINDING(6) uniform PerLightFragment
{
	vec4 u_ambient_color;
    vec4 u_light_pos;
    vec4 u_light_color;
	vec4 u_light_atten;
	vec4 u_spot_light_dir;
	vec4 u_shadow_params;
};
VK_LAYOUT_LOCATION(0) in vec3 v_pos;
VK_LAYOUT_LOCATION(1) in vec2 v_uv;
VK_LAYOUT_LOCATION(2) in vec3 v_normal;
VK_LAYOUT_LOCATION(3) in vec3 v_camera_pos;
layout(location = 0) out vec4 o_color;
void main()
{
	vec3 normal = normalize(v_normal);
	vec3 light_dir = normalize(u_light_pos.xyz - v_pos * u_light_pos.w);
	vec3 view_dir = normalize(v_camera_pos - v_pos);
	vec4 c = texture(u_texture, v_uv);

	float falloff_u = clamp(1.0 - abs(dot(normal, view_dir)), 0.02, 0.98);
	vec4 falloff_c = FALLOFF_POWER * texture(_FalloffSampler, vec2(falloff_u, 0.25));
	vec3 combined = mix(c.rgb, falloff_c.rgb * c.rgb, falloff_c.a);

	float rimlight = clamp(0.5 * (dot(normal, light_dir) + 1.0), 0.0, 1.0);
	falloff_u = clamp(rimlight * falloff_u, 0.0, 1.0);
	falloff_u = texture(_RimLightSampler, vec2(falloff_u, 0.25)).r;
	vec3 light_color = c.rgb * 0.5;
	combined += falloff_u * light_color;

	vec4 final_color = vec4(combined, c.a) * u_color;
	final_color.rgb *= u_light_color.rgb;

#if (LIGHT_ADD_ON == 1)
	c = vec4(0.0);
#else
	c = final_color;
#endif

	o_color = c;
}
]]

return {
	vs = vs,
	fs = fs,
}
