local vs = [[
VK_UNIFORM_BINDING(0) uniform PerView
{
	mat4 u_view_matrix;
    mat4 u_projection_matrix;
	vec4 u_camera_pos;
};
layout(location = 0) in vec4 i_vertex;
VK_LAYOUT_LOCATION(0) out vec3 v_uv;
void main()
{
    mat4 model_matrix = mat4(
		vec4(1, 0, 0, u_camera_pos.x),
		vec4(0, 1, 0, u_camera_pos.y),
		vec4(0, 0, 1, u_camera_pos.z),
		vec4(0, 0, 0, 1));
	gl_Position = (i_vertex * model_matrix * u_view_matrix * u_projection_matrix).xyww;
	v_uv = i_vertex.xyz;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_SAMPLER_BINDING(0) uniform samplerCube u_texture;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
    vec4 u_level;
};
VK_LAYOUT_LOCATION(0) in vec3 v_uv;
layout(location = 0) out vec4 o_color;
void main()
{
	vec4 c = textureLod(u_texture, v_uv, u_level.x);
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
]]

local rs = {
    Cull = Off,
    ZTest = LEqual,
    ZWrite = Off,
    SrcBlendMode = One,
    DstBlendMode = Zero,
	CWrite = On,
    Queue = AlphaTest + 1,
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
			},
		},
        {
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "u_level",
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
