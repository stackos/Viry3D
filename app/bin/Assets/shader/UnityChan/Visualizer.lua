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

VK_LAYOUT_LOCATION(0) out vec4 v_pos_proj;
VK_LAYOUT_LOCATION(1) out vec4 v_pos_world;

void main()
{
    mat4 model_matrix = u_model_matrix;
    vec4 pos_world = i_vertex * model_matrix;
	gl_Position = pos_world * u_view_matrix * u_projection_matrix;
    v_pos_proj = gl_Position;
    v_pos_world = pos_world;

	vk_convert();
}
]]

local fs = [[
precision highp float;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _Center;
};

VK_LAYOUT_LOCATION(0) in vec4 v_pos_proj;
VK_LAYOUT_LOCATION(1) in vec4 v_pos_world;

layout(location = 0) out vec4 o_color;

float rings(vec3 pos)
{
    return 1.0;
}

void main()
{
    vec2 coord = v_pos_proj.xy / v_pos_proj.w * 0.5 + 0.5;

    vec3 center = v_pos_world.xyz - _Center.xyz;
    float trails = rings(center);

	vec4 c = vec4(0.0);
    c = vec4(trails, 0.0, 0.0, 0.0);
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
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "_Center",
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
