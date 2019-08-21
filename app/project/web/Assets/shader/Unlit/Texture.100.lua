local vs = [[
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;
uniform vec4 u_texture_scale_offset;

attribute vec4 i_vertex;
attribute vec2 i_uv;
varying vec2 v_uv;

void main()
{
    mat4 model_matrix = u_model_matrix;

	gl_Position = i_vertex * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv * u_texture_scale_offset.xy + u_texture_scale_offset.zw;

	vk_convert();
}
]]

local fs = [[
precision highp float;
uniform sampler2D u_texture;
uniform vec4 u_color;

varying vec2 v_uv;

void main()
{
	vec4 c = texture2D(u_texture, v_uv) * u_color;
	c.a = 0.0;
	gl_FragColor = c;
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
