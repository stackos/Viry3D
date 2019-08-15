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
VK_SAMPLER_BINDING(0) uniform highp sampler2D u_texture;

float LinearEyeDepth(float z)
{
	return 1.0 / (_ZBufferParams.z * z + _ZBufferParams.w);
}

void main()
{
	float depth = LinearEyeDepth(texture(u_texture, v_uv).r);
    float coc = (depth - _Distance.x) * _LensCoeff.x / max(depth, 1e-5);
	o_color = vec4(clamp(coc * 0.5 * _RcpMaxCoC.x + 0.5, 0.0, 1.0));
}
]]

local fs_prefilter = fs_base .. [[
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
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

local kernel_small = [[
const int kSampleCount = 16;
const vec2 kDiskKernel[kSampleCount] = vec2[](
    vec2(0.0,0.0),
    vec2(0.54545456,0.0),
    vec2(0.16855472,0.5187581),
    vec2(-0.44128203,0.3206101),
    vec2(-0.44128197,-0.3206102),
    vec2(0.1685548,-0.5187581),
    vec2(1.0,0.0),
    vec2(0.809017,0.58778524),
    vec2(0.30901697,0.95105654),
    vec2(-0.30901703,0.9510565),
    vec2(-0.80901706,0.5877852),
    vec2(-1.0,0.0),
    vec2(-0.80901694,-0.58778536),
    vec2(-0.30901664,-0.9510566),
    vec2(0.30901712,-0.9510565),
    vec2(0.80901694,-0.5877853)
);
]]

local kernel_medium = [[
const int kSampleCount = 22;
const vec2 kDiskKernel[kSampleCount] = vec2[](
    vec2(0.0,0.0),
    vec2(0.53333336,0.0),
    vec2(0.3325279,0.4169768),
    vec2(-0.11867785,0.5199616),
    vec2(-0.48051673,0.2314047),
    vec2(-0.48051673,-0.23140468),
    vec2(-0.11867763,-0.51996166),
    vec2(0.33252785,-0.4169769),
    vec2(1.0,0.0),
    vec2(0.90096885,0.43388376),
    vec2(0.6234898,0.7818315),
    vec2(0.22252098,0.9749279),
    vec2(-0.22252095,0.9749279),
    vec2(-0.62349,0.7818314),
    vec2(-0.90096885,0.43388382),
    vec2(-1.0,0.0),
    vec2(-0.90096885,-0.43388376),
    vec2(-0.6234896,-0.7818316),
    vec2(-0.22252055,-0.974928),
    vec2(0.2225215,-0.9749278),
    vec2(0.6234897,-0.7818316),
    vec2(0.90096885,-0.43388376)
);
]]

local kernel_large = [[
const int kSampleCount = 43;
const vec2 kDiskKernel[kSampleCount] = vec2[](
    vec2(0.0,0.0),
    vec2(0.36363637,0.0),
    vec2(0.22672357,0.28430238),
    vec2(-0.08091671,0.35451925),
    vec2(-0.32762504,0.15777594),
    vec2(-0.32762504,-0.15777591),
    vec2(-0.08091656,-0.35451928),
    vec2(0.22672352,-0.2843024),
    vec2(0.6818182,0.0),
    vec2(0.614297,0.29582983),
    vec2(0.42510667,0.5330669),
    vec2(0.15171885,0.6647236),
    vec2(-0.15171883,0.6647236),
    vec2(-0.4251068,0.53306687),
    vec2(-0.614297,0.29582986),
    vec2(-0.6818182,0.0),
    vec2(-0.614297,-0.29582983),
    vec2(-0.42510656,-0.53306705),
    vec2(-0.15171856,-0.66472363),
    vec2(0.1517192,-0.6647235),
    vec2(0.4251066,-0.53306705),
    vec2(0.614297,-0.29582983),
    vec2(1.0,0.0),
    vec2(0.9555728,0.2947552),
    vec2(0.82623875,0.5633201),
    vec2(0.6234898,0.7818315),
    vec2(0.36534098,0.93087375),
    vec2(0.07473,0.9972038),
    vec2(-0.22252095,0.9749279),
    vec2(-0.50000006,0.8660254),
    vec2(-0.73305196,0.6801727),
    vec2(-0.90096885,0.43388382),
    vec2(-0.98883086,0.14904208),
    vec2(-0.9888308,-0.14904249),
    vec2(-0.90096885,-0.43388376),
    vec2(-0.73305184,-0.6801728),
    vec2(-0.4999999,-0.86602545),
    vec2(-0.222521,-0.9749279),
    vec2(0.07473029,-0.99720377),
    vec2(0.36534148,-0.9308736),
    vec2(0.6234897,-0.7818316),
    vec2(0.8262388,-0.56332),
    vec2(0.9555729,-0.29475483)
);
]]

local kernel_very_large = [[
const int kSampleCount = 71;
const vec2 kDiskKernel[kSampleCount] = vec2[](
    vec2(0.0,0.0),
    vec2(0.2758621,0.0),
    vec2(0.1719972,0.21567768),
    vec2(-0.061385095,0.26894566),
    vec2(-0.24854316,0.1196921),
    vec2(-0.24854316,-0.11969208),
    vec2(-0.061384983,-0.2689457),
    vec2(0.17199717,-0.21567771),
    vec2(0.51724136,0.0),
    vec2(0.46601835,0.22442262),
    vec2(0.32249472,0.40439558),
    vec2(0.11509705,0.50427306),
    vec2(-0.11509704,0.50427306),
    vec2(-0.3224948,0.40439552),
    vec2(-0.46601835,0.22442265),
    vec2(-0.51724136,0.0),
    vec2(-0.46601835,-0.22442262),
    vec2(-0.32249463,-0.40439564),
    vec2(-0.11509683,-0.5042731),
    vec2(0.11509732,-0.504273),
    vec2(0.32249466,-0.40439564),
    vec2(0.46601835,-0.22442262),
    vec2(0.7586207,0.0),
    vec2(0.7249173,0.22360738),
    vec2(0.6268018,0.4273463),
    vec2(0.47299224,0.59311354),
    vec2(0.27715522,0.7061801),
    vec2(0.056691725,0.75649947),
    vec2(-0.168809,0.7396005),
    vec2(-0.3793104,0.65698475),
    vec2(-0.55610836,0.51599306),
    vec2(-0.6834936,0.32915324),
    vec2(-0.7501475,0.113066405),
    vec2(-0.7501475,-0.11306671),
    vec2(-0.6834936,-0.32915318),
    vec2(-0.5561083,-0.5159932),
    vec2(-0.37931028,-0.6569848),
    vec2(-0.16880904,-0.7396005),
    vec2(0.056691945,-0.7564994),
    vec2(0.2771556,-0.7061799),
    vec2(0.47299215,-0.59311366),
    vec2(0.62680185,-0.4273462),
    vec2(0.72491735,-0.22360711),
    vec2(1.0,0.0),
    vec2(0.9749279,0.22252093),
    vec2(0.90096885,0.43388376),
    vec2(0.7818315,0.6234898),
    vec2(0.6234898,0.7818315),
    vec2(0.43388364,0.9009689),
    vec2(0.22252098,0.9749279),
    vec2(0.0,1.0),
    vec2(-0.22252095,0.9749279),
    vec2(-0.43388385,0.90096885),
    vec2(-0.62349,0.7818314),
    vec2(-0.7818317,0.62348956),
    vec2(-0.90096885,0.43388382),
    vec2(-0.9749279,0.22252093),
    vec2(-1.0,0.0),
    vec2(-0.9749279,-0.22252087),
    vec2(-0.90096885,-0.43388376),
    vec2(-0.7818314,-0.6234899),
    vec2(-0.6234896,-0.7818316),
    vec2(-0.43388346,-0.900969),
    vec2(-0.22252055,-0.974928),
    vec2(0.0,-1.0),
    vec2(0.2225215,-0.9749278),
    vec2(0.4338835,-0.90096897),
    vec2(0.6234897,-0.7818316),
    vec2(0.78183144,-0.62348986),
    vec2(0.90096885,-0.43388376),
    vec2(0.9749279,-0.22252086)
);
]]

local fs_bokeh = [[
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;

const float PI = 3.14159;

void main()
{
	vec4 samp0 = texture(u_texture, v_uv);

	vec4 bg_acc = vec4(0.0);
	vec4 fg_acc = vec4(0.0);

	for (int si = 0; si < kSampleCount; si++)
	{
		vec2 disp = kDiskKernel[si] * _MaxCoC.x;
        float dist = length(disp);

        vec2 duv = vec2(disp.x * _RcpAspect.x, disp.y);
        vec4 samp = texture(u_texture, v_uv + duv);

		float bg_coc = max(min(samp0.a, samp.a), 0.0);

        float margin = u_texel_size.y * 2.0;
        float bg_weight = clamp((bg_coc  - dist + margin) / margin, 0.0, 1.0);
        float fg_weight = clamp((-samp.a - dist + margin) / margin, 0.0, 1.0);

        fg_weight *= step(u_texel_size.y, -samp.a);

        bg_acc += vec4(samp.rgb, 1.0) * bg_weight;
        fg_acc += vec4(samp.rgb, 1.0) * fg_weight;
	}

	bg_acc.rgb /= bg_acc.a + float(bg_acc.a == 0.0);
    fg_acc.rgb /= fg_acc.a + float(fg_acc.a == 0.0);

	bg_acc.a = smoothstep(u_texel_size.y, u_texel_size.y * 2.0, samp0.a);
    fg_acc.a *= PI / float(kSampleCount);

    float alpha = clamp(fg_acc.a, 0.0, 1.0);
    vec3 rgb = mix(bg_acc.rgb, fg_acc.rgb, alpha);

	o_color = vec4(rgb, alpha);
}
]]

local fs_bokeh_small = fs_base .. kernel_small .. fs_bokeh
local fs_bokeh_medium = fs_base .. kernel_medium .. fs_bokeh
local fs_bokeh_large = fs_base .. kernel_large .. fs_bokeh
local fs_bokeh_very_large = fs_base .. kernel_very_large .. fs_bokeh

local fs_postfilter = fs_base .. [[
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;

void main()
{
	vec4 duv = u_texel_size.xyxy * vec4(0.5, 0.5, -0.5, 0.0);
    vec4 acc = texture(u_texture, v_uv - duv.xy);
    acc		+= texture(u_texture, v_uv - duv.zy);
    acc		+= texture(u_texture, v_uv + duv.zy);
    acc		+= texture(u_texture, v_uv + duv.xy);
    o_color = acc / 4.0;
}
]]

local fs_combine = fs_base .. [[
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_SAMPLER_BINDING(1) uniform sampler2D _CoCTex;
VK_SAMPLER_BINDING(2) uniform sampler2D _DofTex;

void main()
{
	vec4 dof = texture(_DofTex, v_uv);
    float coc = texture(_CoCTex, v_uv).r;
    coc = (coc - 0.5) * 2.0 * _MaxCoC.x;

    float ffa = smoothstep(u_texel_size.y * 2.0, u_texel_size.y * 4.0, coc);

    vec4 color = texture(u_texture, v_uv);
    float alpha = Max3(dof.r, dof.g, dof.b);
    o_color = mix(color, vec4(dof.rgb, alpha), ffa + dof.a - ffa * dof.a);
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

local samplers_combine = {
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
			{
				name = "_DofTex",
				binding = 2,
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

local pass_bokeh_small = {
    vs = vs,
    fs = fs_bokeh_small,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_bokeh_medium = {
    vs = vs,
    fs = fs_bokeh_medium,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_bokeh_large = {
    vs = vs,
    fs = fs_bokeh_large,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_bokeh_very_large = {
    vs = vs,
    fs = fs_bokeh_very_large,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_postfilter = {
    vs = vs,
    fs = fs_postfilter,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_combine = {
    vs = vs,
    fs = fs_combine,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_combine,
}

-- return pass array
return {
    pass_coc,
	pass_prefilter,
	pass_bokeh_small,
	pass_bokeh_medium,
	pass_bokeh_large,
	pass_bokeh_very_large,
	pass_postfilter,
	pass_combine,
}
