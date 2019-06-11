local vs = [[
#ifndef SKIN_ON
	#define SKIN_ON 0
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

	vk_convert();
}
]]

local fs = [[
#ifndef LIGHT_ADD_ON
	#define LIGHT_ADD_ON 0
#endif

precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_UNIFORM_BINDING(5) uniform PerLight
{
	vec4 u_ambient_color;
    vec4 u_light_pos;
    vec4 u_light_color;
	vec4 u_light_atten;
	vec4 u_spot_light_dir;
};
VK_LAYOUT_LOCATION(0) in vec3 v_pos;
VK_LAYOUT_LOCATION(1) in vec2 v_uv;
VK_LAYOUT_LOCATION(2) in vec3 v_normal;
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
            name = "PerLight",
            binding = 5,
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
            },
        },
	},
	samplers = {
		{
			name = "u_texture",
			binding = 0,
		},
	},
}

-- return pass array
return {
    pass
}
