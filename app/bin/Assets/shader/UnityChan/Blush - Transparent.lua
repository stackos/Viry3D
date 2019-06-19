local CharaSkin = require("CharaSkin")

local vs = CharaSkin.vs

local fs = CharaSkin.fs

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
    ZWrite = Off,
    SrcBlendMode = SrcAlpha,
    DstBlendMode = OneMinusSrcAlpha,
	CWrite = On,
    Queue = Geometry + 3,
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
				{
                    name = "u_camera_pos",
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
            name = "PerMaterialFragment",
            binding = 4,
            members = {
                {
                    name = "u_color",
                    size = 16,
                },
				{
                    name = "_ShadowColor",
                    size = 16,
                },
            },
        },
		{
            name = "PerLightFragment",
            binding = 6,
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
				{
                    name = "u_shadow_params",
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
					name = "_FalloffSampler",
					binding = 1,
				},
				{
					name = "_RimLightSampler",
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
