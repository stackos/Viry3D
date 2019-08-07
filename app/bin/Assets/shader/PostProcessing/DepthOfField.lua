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
	vec4 u_texel_size;
	vec4 _ZBufferParams;
	vec4 _Distance;
	vec4 _LensCoeff;
	vec4 _MaxCoC;
	vec4 _RcpMaxCoC;
	vec4 _RcpAspect;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;

float Max3(float a, float b, float c)
{
	return max(max(a, b), c);
}

float Min3(float a, float b, float c)
{
	return min(min(a, b), c);
}
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

local fs_prefilter = fs_base .. [[
VK_SAMPLER_BINDING(1) uniform sampler2D _CoCTex;

void main()
{
	vec3 duv = u_texel_size.xyx * vec3(0.5, 0.5, -0.5);
    vec2 uv0 = v_uv - duv.xy;
    vec2 uv1 = v_uv - duv.zy;
    vec2 uv2 = v_uv + duv.zy;
    vec2 uv3 = v_uv + duv.xy;

    vec3 c0 = texture(u_texture, uv0).rgb;
    vec3 c1 = texture(u_texture, uv1).rgb;
    vec3 c2 = texture(u_texture, uv2).rgb;
    vec3 c3 = texture(u_texture, uv3).rgb;

    float coc0 = texture(_CoCTex, uv0).r * 2.0 - 1.0;
    float coc1 = texture(_CoCTex, uv1).r * 2.0 - 1.0;
    float coc2 = texture(_CoCTex, uv2).r * 2.0 - 1.0;
    float coc3 = texture(_CoCTex, uv3).r * 2.0 - 1.0;

    float w0 = abs(coc0) / (Max3(c0.r, c0.g, c0.b) + 1.0);
    float w1 = abs(coc1) / (Max3(c1.r, c1.g, c1.b) + 1.0);
    float w2 = abs(coc2) / (Max3(c2.r, c2.g, c2.b) + 1.0);
    float w3 = abs(coc3) / (Max3(c3.r, c3.g, c3.b) + 1.0);

    vec3 avg = c0 * w0 + c1 * w1 + c2 * w2 + c3 * w3;
    avg /= max(w0 + w1 + w2 + w3, 1e-4);

    float coc_min = min(coc0, Min3(coc1, coc2, coc3));
    float coc_max = max(coc0, Max3(coc1, coc2, coc3));
    float coc = (-coc_min > coc_max ? coc_min : coc_max) * _MaxCoC.x;

    avg *= smoothstep(0.0, u_texel_size.y * 2.0, abs(coc));

    o_color = vec4(avg, coc);
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
                name = "u_texel_size",
                size = 16,
            },
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

local samplers_prefilter = {
	{
		name = "PerMaterialFragment",
		binding = 4,
		samplers = {
			{
				name = "u_texture",
				binding = 0,
			},
			{
				name = "_CoCTex",
				binding = 1,
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

local pass_prefilter = {
    vs = vs,
    fs = fs_prefilter,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_prefilter,
}

-- return pass array
return {
    pass_coc,
	pass_prefilter,
}
