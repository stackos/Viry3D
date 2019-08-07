local vs = [[
layout(location = 0) in vec4 i_vertex;
layout(location = 2) in vec2 i_uv;
VK_LAYOUT_LOCATION(0) out vec2 v_uv;
void main()
{
	gl_Position = i_vertex;
	v_uv = i_uv;

	vk_convert();
}
]]

local fs_base = [[
precision highp float;
VK_SAMPLER_BINDING(0) uniform highp sampler2D u_texture;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _ZBufferParams;
	vec4 _Distance;
	vec4 _LensCoeff;
	vec4 _MaxCoC;
	vec4 _RcpMaxCoC;
	vec4 _RcpAspect;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;
]]

local fs_coc = fs_base .. [[
float LinearEyeDepth(float z)
{
	return 1.0 / (_ZBufferParams.z * z + _ZBufferParams.w);
}

void main()
{
	float depth = LinearEyeDepth(texture(u_texture, v_uv).r);
    float coc = (depth - _Distance.x) * _LensCoeff.x / max(depth, 1e-4);
	o_color = vec4(clamp(coc * 0.5 * _RcpMaxCoC.x + 0.5, 0.0, 1.0));
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
    SrcBlendMode = One,
    DstBlendMode = Zero,
	CWrite = On,
    Queue = Overlay,
}

local uniforms_base = {
	{
        name = "PerMaterialFragment",
        binding = 4,
        members = {
			{
                name = "_ZBufferParams",
                size = 16,
            },
            {
                name = "_Distance",
                size = 16,
            },
			{
                name = "_LensCoeff",
                size = 16,
            },
			{
                name = "_MaxCoC",
                size = 16,
            },
            {
                name = "_RcpMaxCoC",
                size = 16,
            },
            {
                name = "_RcpAspect",
                size = 16,
            },
        },
    },
}

local samplers_base = {
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
}

local pass_coc = {
    vs = vs,
    fs = fs_coc,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

-- return pass array
return {
    pass_coc,
}
