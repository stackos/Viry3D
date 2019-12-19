local vs = [[
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;
uniform mat4 u_bounds_matrix;

attribute vec4 i_vertex;

void main()
{
	gl_Position = vec4(i_vertex.xyz, 1.0) * u_bounds_matrix * u_model_matrix * u_view_matrix * u_projection_matrix;

	vk_convert();
}
]]

local fs = [[
precision highp float;
void main()
{
	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
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
