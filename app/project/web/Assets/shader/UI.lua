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
VK_UNIFORM_BINDING(3) uniform PerMaterialVertex
{
	vec4 u_texture_scale_offset;
};
layout(location = 0) in vec4 i_vertex;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2 i_uv;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;
VK_LAYOUT_LOCATION(1) out vec4 v_color;
void main()
{
    mat4 model_matrix = u_model_matrix;
	gl_Position = i_vertex * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv * u_texture_scale_offset.xy + u_texture_scale_offset.zw;
	v_color = i_color;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 u_color;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
VK_LAYOUT_LOCATION(1) in vec4 v_color;
layout(location = 0) out vec4 o_color;
void main()
{
	o_color = texture(u_texture, v_uv) * v_color * u_color;
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
    ZTest = Always,
    ZWrite = Off,
    SrcBlendMode = SrcAlpha,
    DstBlendMode = OneMinusSrcAlpha,
	CWrite = On,
    Queue = Overlay,
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
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "u_color",
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
	},
}

-- return pass array
return {
    pass
}
