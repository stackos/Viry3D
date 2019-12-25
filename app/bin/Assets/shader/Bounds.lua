local vs = [[
VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
};
VK_UNIFORM_BINDING(1) uniform PerRenderer
{
	mat4 u_model_matrix;
    mat4 u_bounds_matrix;
    vec4 u_bounds_color;
};
layout(location = 0) in vec4 i_vertex;
VK_LAYOUT_LOCATION(0) out vec4 v_color;
void main()
{
	gl_Position = vec4(i_vertex.xyz, 1.0) * u_bounds_matrix * u_model_matrix * u_view_matrix * u_projection_matrix;
    v_color = u_bounds_color;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_LAYOUT_LOCATION(0) in vec4 v_color;
layout(location = 0) out vec4 o_color;
void main()
{
	o_color = v_color;
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
    ZWrite = On,
    SrcBlendMode = One,
    DstBlendMode = Zero,
	CWrite = On,
    Queue = Geometry,
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
                {
					name = "u_bounds_matrix",
					size = 64,
				},
                {
					name = "u_bounds_color",
					size = 16,
				},
			},
		},
	},
	samplers = {
	},
}

-- return pass array
return {
    pass
}
