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
layout(location = 5) in vec4 i_tangent;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;
VK_LAYOUT_LOCATION(1) out vec3 v_camera_pos;
VK_LAYOUT_LOCATION(2) out vec4 v_tangent_to_world[3];

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
	v_uv = i_uv * u_texture_scale_offset.xy + u_texture_scale_offset.zw;
	v_camera_pos = u_camera_pos.xyz;
	
	vec3 normal = normalize((vec4(i_normal, 0.0) * model_matrix).xyz);
	vec3 tangent = normalize((vec4(i_tangent.xyz, 0.0) * model_matrix).xyz);
    vec3 binormal = cross(normal, tangent) * i_tangent.w;

    v_tangent_to_world[0].xyz = tangent;
    v_tangent_to_world[1].xyz = binormal;
    v_tangent_to_world[2].xyz = normal;
    v_tangent_to_world[0].w = world_pos.x;
    v_tangent_to_world[1].w = world_pos.y;
    v_tangent_to_world[2].w = world_pos.z;

	vk_convert();
}
]]

local fs = [[
#ifndef LIGHT_ADD_ON
	#define LIGHT_ADD_ON 0
#endif

#define FALLOFF_POWER 0.3
precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_SAMPLER_BINDING(1) uniform sampler2D _FalloffSampler;
VK_SAMPLER_BINDING(2) uniform sampler2D _RimLightSampler;
VK_SAMPLER_BINDING(3) uniform sampler2D _SpecularReflectionSampler;
VK_SAMPLER_BINDING(4) uniform sampler2D _EnvMapSampler;
VK_SAMPLER_BINDING(5) uniform sampler2D _NormalMapSampler;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 u_color;
	vec4 _ShadowColor;
	vec4 _SpecularPower;
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
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
VK_LAYOUT_LOCATION(1) in vec3 v_camera_pos;
VK_LAYOUT_LOCATION(2) in vec4 v_tangent_to_world[3];
layout(location = 0) out vec4 o_color;

vec3 normal_map(vec3 pos)
{
	vec3 tangent = normalize(v_tangent_to_world[0].xyz);
    vec3 binormal = normalize(v_tangent_to_world[1].xyz);
    vec3 normal = normalize(v_tangent_to_world[2].xyz);
    vec3 normal_tangent = normalize(texture(_NormalMapSampler, v_uv).xyz * 2.0 - 1.0);
    return normalize(tangent * normal_tangent.x + binormal * normal_tangent.y + normal * normal_tangent.z);
}

vec4 lit(float nl, float nh, float m)
{
	float ambient = 1.0;
	float diffuse = max(nl, 0.0);
	float specular = pow(clamp(nh, 0.0, 1.0), m);
	return vec4(ambient, diffuse, specular, 1.0);
}

vec3 overlay(vec3 upper, vec3 lower)
{
	vec3 minus_lower = vec3(1.0, 1.0, 1.0) - lower;
	vec3 val = 2.0 * minus_lower;
	vec3 min_v = 2.0 * lower - vec3(1.0, 1.0, 1.0);
	vec3 greater = upper * val + min_v;
	vec3 lower_r = 2.0 * lower * upper;

	vec3 lerp_v = round(lower);
	return mix(lower_r, greater, lerp_v);
}

void main()
{
	vec3 pos = vec3(v_tangent_to_world[0].w, v_tangent_to_world[1].w, v_tangent_to_world[2].w);
	vec3 normal = normal_map(pos);
	vec3 light_dir = normalize(u_light_pos.xyz - pos * u_light_pos.w);
	vec3 view_dir = normalize(v_camera_pos - pos);
	float nv = dot(normal, view_dir);
	vec4 c = texture(u_texture, v_uv);

	float falloff_u = clamp(1.0 - abs(nv), 0.02, 0.98);
	vec4 falloff_c = FALLOFF_POWER * texture(_FalloffSampler, vec2(falloff_u, 0.25));
	vec3 combined = mix(c.rgb, c.rgb * c.rgb, falloff_c.r);
	combined *= (1.0 + falloff_c.rgb * falloff_c.a);

	vec4 reflection_mask = texture(_SpecularReflectionSampler, v_uv);
	vec4 spec = lit(nv, nv, _SpecularPower.x);
	vec3 spec_c = clamp(spec.z, 0.0, 1.0) * reflection_mask.rgb * c.rgb;
	combined += spec_c;

	vec3 refl = reflect(-view_dir, normal).xzy;
	vec2 sphere_uv = 0.5 * (vec2(1.0, 1.0) + refl.xy);
	vec3 reflect_c = texture(_EnvMapSampler, sphere_uv).rgb;
	reflect_c = overlay(reflect_c, combined);
	combined = mix(combined, reflect_c, reflection_mask.a);

	combined *= u_color.rgb * u_light_color.rgb;
	float opacity = c.a * u_color.a * u_light_color.a;

	float rimlight = clamp(0.5 * (dot(normal, light_dir) + 1.0), 0.0, 1.0);
	falloff_u = clamp(rimlight * falloff_u, 0.0, 1.0);
	falloff_u = texture(_RimLightSampler, vec2(falloff_u, 0.25)).r;
	vec3 light_color = c.rgb;
	combined += falloff_u * light_color;

	vec4 final_color = vec4(combined, opacity);

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
