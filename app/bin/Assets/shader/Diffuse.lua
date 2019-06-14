local vs = [[
#ifndef SKIN_ON
	#define SKIN_ON 0
#endif
#ifndef RECIEVE_SHADOW_ON
	#define RECIEVE_SHADOW_ON 0
#endif

VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
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

#if (RECIEVE_SHADOW_ON == 1)
	VK_LAYOUT_LOCATION(3) out vec4 v_pos_light_proj;
	VK_UNIFORM_BINDING(5) uniform PerLightVertex
	{
		mat4 u_light_view_matrix;
		mat4 u_light_projection_matrix;
	};
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

#if (RECIEVE_SHADOW_ON == 1)
	v_pos_light_proj = i_vertex * model_matrix * u_light_view_matrix * u_light_projection_matrix;
#endif

	vk_convert();
}
]]

local fs = [[
#ifndef LIGHT_ADD_ON
	#define LIGHT_ADD_ON 0
#endif
#ifndef RECIEVE_SHADOW_ON
	#define RECIEVE_SHADOW_ON 0
#endif
#ifndef VR_GLES
	#define VR_GLES 0
#endif

precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
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

#if (RECIEVE_SHADOW_ON == 1)
	VK_SAMPLER_BINDING(1) uniform highp sampler2D u_shadow_texture;
	VK_LAYOUT_LOCATION(3) in vec4 v_pos_light_proj;
	const vec2 Poisson25[25] = vec2[](
		vec2(-0.978698, -0.0884121),
		vec2(-0.841121, 0.521165),
		vec2(-0.71746, -0.50322),
		vec2(-0.702933, 0.903134),
		vec2(-0.663198, 0.15482),
		vec2(-0.495102, -0.232887),
		vec2(-0.364238, -0.961791),
		vec2(-0.345866, -0.564379),
		vec2(-0.325663, 0.64037),
		vec2(-0.182714, 0.321329),
		vec2(-0.142613, -0.0227363),
		vec2(-0.0564287, -0.36729),
		vec2(-0.0185858, 0.918882),
		vec2(0.0381787, -0.728996),
		vec2(0.16599, 0.093112),
		vec2(0.253639, 0.719535),
		vec2(0.369549, -0.655019),
		vec2(0.423627, 0.429975),
		vec2(0.530747, -0.364971),
		vec2(0.566027, -0.940489),
		vec2(0.639332, 0.0284127),
		vec2(0.652089, 0.669668),
		vec2(0.773797, 0.345012),
		vec2(0.968871, 0.840449),
		vec2(0.991882, -0.657338)
	);
	float texture_shadow(vec2 uv)
	{
		if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
		{
			return 1.0;
		}
		else
		{
			return texture(u_shadow_texture, uv).r;
		}
	}
	float poisson_filter(float z, vec2 uv, float shadow_z_bias, vec2 filter_radius)
	{
		float shadow = 0.0;
		for (int i = 0; i < 25; ++i)
		{
			vec2 offset = Poisson25[i] * filter_radius;
			float shadow_depth = texture_shadow(uv + offset);
			if (z - shadow_z_bias > shadow_depth)
			{
				shadow += 1.0;
			}
		}
		return shadow / 25.0;
	}
	float pcf_filter(float z, vec2 uv, float shadow_z_bias, vec2 filter_radius)
	{
		float shadow = 0.0;
		for (int i = -1; i <= 1; ++i)
		{
			for (int j = -1; j <= 1; ++j)
			{
				vec2 offset = vec2(i, j) * filter_radius;
				float shadow_depth = texture_shadow(uv + offset);
				if (z - shadow_z_bias > shadow_depth)
				{
					shadow += 1.0;
				}
			}
		}
		return shadow / 9.0;
	}
	float linear_filter(float z, vec2 uv, float shadow_z_bias, vec2 filter_radius)
	{
		float shadow_depth = texture_shadow(uv);
		if (z - shadow_z_bias > shadow_depth)
		{
			return 1.0;
		}
		else
		{
			return 0.0;
		}
	}
	float sample_shadow(vec4 pos_light_proj, float nl)
	{
		pos_light_proj = pos_light_proj / pos_light_proj.w;
		vec2 uv = pos_light_proj.xy * 0.5 + 0.5;
#if (VR_GLES == 0)
		uv.y = 1.0 - uv.y;
#endif
		float z = pos_light_proj.z * 0.5 + 0.5;
		vec2 filter_radius = vec2(u_shadow_params.w);
		float shadow_z_bias = u_shadow_params.y + u_shadow_params.z * tan(acos(nl));
		return poisson_filter(z, uv, shadow_z_bias, filter_radius) * u_shadow_params.x;
	}
#endif

layout(location = 0) out vec4 o_color;
void main()
{
    vec3 normal = normalize(v_normal);
	vec3 to_light = u_light_pos.xyz - v_pos * u_light_pos.w;
	vec3 light_dir = normalize(to_light);
    float nl = max(dot(normal, light_dir), 0.0);
	vec4 c = texture(u_texture, v_uv);

	float sqr_len = dot(to_light, to_light);
	float atten = 1.0 - sqr_len * u_light_atten.z;
	int light_type = int(u_light_color.a);
	if (light_type == 1)
	{
		float theta = dot(light_dir, u_spot_light_dir.xyz);
		if (theta > u_light_atten.x)
		{
			atten *= clamp((u_light_atten.x - theta) * u_light_atten.y, 0.0, 1.0);
		}
		else
		{
			atten = 0.0;
		}
	}

	vec3 diffuse = c.rgb * nl * u_light_color.rgb * atten;

#if (RECIEVE_SHADOW_ON == 1)
	float shadow = sample_shadow(v_pos_light_proj, nl);
    diffuse = diffuse * (1.0 - shadow);
#endif

#if (LIGHT_ADD_ON == 1)
	c.rgb = diffuse;
#else
	vec3 ambient = c.rgb * u_ambient_color.rgb;
	c.rgb = ambient + diffuse;
#endif

	c.a = 1.0;
    o_color = c;
}
]]

--[[
    Cull
	    Back | Front | Off
    ZTest
	    Less | Greater | LEqual | GEqual | Equal | NotEqual | Always
    ZWrite
	    On | Off
    SrcBlendMode
	DstBlendMode
	    One | Zero | SrcColor | SrcAlpha | DstColor | DstAlpha
		| OneMinusSrcColor | OneMinusSrcAlpha | OneMinusDstColor | OneMinusDstAlpha
	CWrite
		On | Off
	Queue
		Background | Geometry | AlphaTest | Transparent | Overlay
	LightMode
		None | Forward
]]

local rs = {
    Cull = Back,
    ZTest = LEqual,
    ZWrite = On,
    SrcBlendMode = One,
    DstBlendMode = Zero,
	CWrite = On,
    Queue = Geometry,
	LightMode = Forward,
}

local pass = {
    vs = vs,
    fs = fs,
    rs = rs,
	uniforms = {
		{
			name = "PerView",
			binding = 0,
			members = {
				{
					name = "u_view_matrix",
					size = 64,
				},
                {
                    name = "u_projection_matrix",
                    size = 64,
                },
			},
		},
		{
			name = "PerRenderer",
			binding = 1,
			members = {
				{
					name = "u_model_matrix",
					size = 64,
				},
			},
		},
        {
            name = "PerRendererBones",
            binding = 2,
            members = {
                {
                    name = "u_bones",
                    size = 16 * 210,
                },
            },
        },
		{
            name = "PerMaterialVertex",
            binding = 3,
            members = {
                {
                    name = "u_texture_scale_offset",
                    size = 16,
                },
            },
        },
		{
            name = "PerLightVertex",
            binding = 5,
            members = {
				{
					name = "u_light_view_matrix",
					size = 64,
				},
                {
                    name = "u_light_projection_matrix",
                    size = 64,
                },
			},
        },
        {
            name = "PerLightFragment",
            binding = 6,
            members = {
				{
                    name = "u_ambient_color",
                    size = 16,
                },
                {
                    name = "u_light_pos",
                    size = 16,
                },
                {
                    name = "u_light_color",
                    size = 16,
                },
				{
                    name = "u_light_atten",
                    size = 16,
                },
				{
                    name = "u_spot_light_dir",
                    size = 16,
                },
				{
                    name = "u_shadow_params",
                    size = 16,
                },
            },
        },
	},
	samplers = {
		{
			name = "PerMaterialFragment",
			binding = 4,
			samplers = {
				{
					name = "u_texture",
					binding = 0,
				},
			},
		},
		{
			name = "PerLightFragment",
			binding = 6,
			samplers = {
				{
					name = "u_shadow_texture",
					binding = 1,
				},
			},
		},
	},
}

-- return pass array
return {
    pass
}
