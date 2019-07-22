local vs = [[
VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
	vec4 u_camera_pos;
	vec4 u_time;
};
VK_UNIFORM_BINDING(1) uniform PerRenderer
{
	mat4 u_model_matrix;
};
VK_UNIFORM_BINDING(3) uniform PerMaterialVertex
{
	vec4 _NoiseScale;
	vec4 _NoiseSpeed;
};
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
layout(location = 4) in vec3 i_normal;
VK_LAYOUT_LOCATION(0) out vec3 v_pos;
VK_LAYOUT_LOCATION(1) out vec2 v_uv;
VK_LAYOUT_LOCATION(2) out vec3 v_normal;
VK_LAYOUT_LOCATION(3) out vec2 v_uv1;
VK_LAYOUT_LOCATION(4) out vec2 v_uv2;
VK_LAYOUT_LOCATION(5) out vec3 v_cam_pos;

void main()
{
    mat4 model_matrix = u_model_matrix;
	vec4 world_pos = i_vertex * model_matrix;
	gl_Position = world_pos * u_view_matrix * u_projection_matrix;
	v_pos = world_pos.xyz;
	v_uv = i_uv;
    v_normal = (vec4(i_normal, 0.0) * model_matrix).xyz;

	v_uv1 = world_pos.xy * _NoiseScale.xy + _NoiseSpeed.xy * u_time.y;
    v_uv2 = world_pos.xy * _NoiseScale.zw + _NoiseSpeed.zw * u_time.y;
	v_cam_pos = u_camera_pos.xyz;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D _MainTex;
VK_SAMPLER_BINDING(1) uniform sampler2D _NoiseTex1;
VK_SAMPLER_BINDING(2) uniform sampler2D _NoiseTex2;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _Color;
};
VK_LAYOUT_LOCATION(0) in vec3 v_pos;
VK_LAYOUT_LOCATION(1) in vec2 v_uv;
VK_LAYOUT_LOCATION(2) in vec3 v_normal;
VK_LAYOUT_LOCATION(3) in vec2 v_uv1;
VK_LAYOUT_LOCATION(4) in vec2 v_uv2;
VK_LAYOUT_LOCATION(5) in vec3 v_cam_pos;
layout(location = 0) out vec4 o_color;

void main()
{
	vec3 normal = v_normal;
	vec3 camDir = normalize(v_pos - v_cam_pos);
	float falloff = max(abs(dot(camDir, normal)) - 0.4, 0.0);
	falloff = falloff * falloff * 5.0;

	vec4 c = _Color;

    float n1 = texture(_NoiseTex1, v_uv1).r;
    float n2 = texture(_NoiseTex2, v_uv2).r;

    c.a *= texture(_MainTex, v_uv).a * n1 * n2 * falloff;

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
]]

local rs = {
    Cull = Off,
    ZTest = LEqual,
    ZWrite = Off,
    SrcBlendMode = SrcAlpha,
    DstBlendMode = OneMinusSrcAlpha,
	CWrite = On,
    Queue = Transparent,
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
				{
                    name = "u_camera_pos",
                    size = 16,
                },
				{
                    name = "u_time",
                    size = 16,
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
            name = "PerMaterialVertex",
            binding = 3,
            members = {
                {
                    name = "_NoiseScale",
                    size = 16,
                },
				{
                    name = "_NoiseSpeed",
                    size = 16,
                },
            },
        },
		{
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "_Color",
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
					name = "_MainTex",
					binding = 0,
				},
				{
					name = "_NoiseTex1",
					binding = 1,
				},
				{
					name = "_NoiseTex2",
					binding = 2,
				},
			},
		},
	},
}

-- return pass array
return {
    pass
}
