local vs = [[
VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
};
VK_UNIFORM_BINDING(1) uniform PerRenderer
{
	mat4 u_model_matrix;
};
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;

void main()
{
    mat4 model_matrix = u_model_matrix;
	gl_Position = i_vertex * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D _MainTex;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _Color;
	vec4 _Cutoff;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;
void main()
{
	vec4 c = texture(_MainTex, v_uv) * _Color;
	if (c.a < _Cutoff.x)
	{
		discard;
	}
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
    Cull = Back,
    ZTest = LEqual,
    ZWrite = On,
    SrcBlendMode = One,
    DstBlendMode = Zero,
	CWrite = On,
    Queue = AlphaTest,
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
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "_Color",
                    size = 16,
                },
				{
                    name = "_Cutoff",
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
			},
		},
	},
}

-- return pass array
return {
    pass
}
