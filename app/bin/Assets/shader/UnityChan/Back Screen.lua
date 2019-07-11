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
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;
VK_LAYOUT_LOCATION(1) out vec4 v_time;

void main()
{
    mat4 model_matrix = u_model_matrix;
	gl_Position = i_vertex * model_matrix * u_view_matrix * u_projection_matrix;
	v_uv = i_uv;
    v_time = u_time;

	vk_convert();
}
]]

local fs = [[
#ifndef VR_GLES
#define VR_GLES 0
#endif

precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_SAMPLER_BINDING(1) uniform sampler2D _StripeTex;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _Params;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
VK_LAYOUT_LOCATION(1) in vec4 v_time;
layout(location = 0) out vec4 o_color;
void main()
{
#if (VR_GLES == 1)
    vec2 uv0 = vec2(v_uv.x, 1.0 - v_uv.y);
#else
	vec2 uv0 = v_uv;
#endif
    vec2 uv1 = v_uv * vec2(4.0, 4.0);

    vec4 c = texture(u_texture, uv0);
	float amp = texture(_StripeTex, uv1).r;
    amp = _Params.x + _Params.y * amp;

    float time = v_time.y * 3.14 * _Params.w;
    float flicker = mix(1.0, sin(time) * 0.5, _Params.z);

    o_color = c * (amp * flicker);
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
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "_Params",
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
				{
					name = "_StripeTex",
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
