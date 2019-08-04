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
VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 u_texel_size;
	vec4 _Params;
	vec4 _Threshold;
    vec4 _SampleScale;
    vec4 _Bloom_Settings;
    vec4 _Bloom_Color;
};
VK_LAYOUT_LOCATION(0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;

vec4 DownsampleBox13Tap(vec2 uv, vec2 texel_size)
{
    vec4 A = texture(u_texture, (uv + texel_size * vec2(-1.0, -1.0)));
    vec4 B = texture(u_texture, (uv + texel_size * vec2( 0.0, -1.0)));
    vec4 C = texture(u_texture, (uv + texel_size * vec2( 1.0, -1.0)));
    vec4 D = texture(u_texture, (uv + texel_size * vec2(-0.5, -0.5)));
    vec4 E = texture(u_texture, (uv + texel_size * vec2( 0.5, -0.5)));
    vec4 F = texture(u_texture, (uv + texel_size * vec2(-1.0,  0.0)));
    vec4 G = texture(u_texture, (uv								  ));
    vec4 H = texture(u_texture, (uv + texel_size * vec2( 1.0,  0.0)));
    vec4 I = texture(u_texture, (uv + texel_size * vec2(-0.5,  0.5)));
    vec4 J = texture(u_texture, (uv + texel_size * vec2( 0.5,  0.5)));
    vec4 K = texture(u_texture, (uv + texel_size * vec2(-1.0,  1.0)));
    vec4 L = texture(u_texture, (uv + texel_size * vec2( 0.0,  1.0)));
    vec4 M = texture(u_texture, (uv + texel_size * vec2( 1.0,  1.0)));

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

vec4 UpsampleTent(sampler2D tex, vec2 uv, vec2 texel_size, float sample_scale)
{
    vec4 d = texel_size.xyxy * vec4(1.0, 1.0, -1.0, 0.0) * sample_scale;

    vec4 s = texture(tex, uv - d.xy);
    s += texture(tex, uv - d.wy) * 2.0;
    s += texture(tex, uv - d.zy);

    s += texture(tex, uv + d.zw) * 2.0;
    s += texture(tex, uv       ) * 4.0;
    s += texture(tex, uv + d.xw) * 2.0;

    s += texture(tex, uv + d.zy);
    s += texture(tex, uv + d.wy) * 2.0;
    s += texture(tex, uv + d.xy);

    return s * (1.0 / 16.0);
}
]]

local fs_prefilter = fs_base .. [[
#define EPSILON 1.0e-4

vec4 QuadraticThreshold(vec4 color, float threshold, vec3 curve)
{
    // Pixel brightness
    float br = max(max(color.r, color.g), color.b);

    // Under-threshold part: quadratic curve
    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    // Combine and apply the brightness response curve.
    color *= max(rq, br - threshold) / max(br, EPSILON);

    return color;
}

vec4 Prefilter(vec4 color)
{
    color = min(_Params.xxxx, color); // clamp to max
    color = QuadraticThreshold(color, _Threshold.x, _Threshold.yzw);
    return color;
}

void main()
{
	vec4 c = DownsampleBox13Tap(v_uv, u_texel_size.xy);
	c = Prefilter(c);
	o_color = c;
}
]]

local fs_downsample = fs_base .. [[
void main()
{
	vec4 c = DownsampleBox13Tap(v_uv, u_texel_size.xy);
	o_color = c;
}
]]

local fs_upsample = fs_base .. [[
VK_SAMPLER_BINDING(1) uniform sampler2D _BloomTex;

vec4 Combine(vec4 bloom, vec2 uv)
{
    vec4 color = texture(_BloomTex, uv);
    return bloom + color;
}

void main()
{
    vec4 bloom = UpsampleTent(u_texture, v_uv, u_texel_size.xy, _SampleScale.x);
    o_color = Combine(bloom, v_uv);
}
]]

local fs_uber = fs_base .. [[
VK_SAMPLER_BINDING(1) uniform sampler2D _BloomTex;

void main()
{
    vec4 color = texture(u_texture, v_uv);
    vec4 bloom = UpsampleTent(_BloomTex, v_uv, u_texel_size.xy, _Bloom_Settings.x);

    bloom *= _Bloom_Settings.y;
    color += bloom * _Bloom_Color;

    o_color = color;
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
                name = "_Params",
                size = 16,
            },
			{
                name = "_Threshold",
                size = 16,
            },
            {
                name = "_SampleScale",
                size = 16,
            },
            {
                name = "_Bloom_Settings",
                size = 16,
            },
            {
                name = "_Bloom_Color",
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

local samplers_upsample = {
    {
        name = "PerMaterialFragment",
        binding = 4,
        samplers = {
            {
                name = "u_texture",
                binding = 0,
            },
            {
                name = "_BloomTex",
                binding = 1,
            },
        },
    },
}

local pass_prefilter = {
    vs = vs,
    fs = fs_prefilter,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_downsample = {
    vs = vs,
    fs = fs_downsample,
    rs = rs,
	uniforms = uniforms_base,
	samplers = samplers_base,
}

local pass_upsample = {
    vs = vs,
    fs = fs_upsample,
    rs = rs,
    uniforms = uniforms_base,
    samplers = samplers_upsample,
}

local pass_uber = {
    vs = vs,
    fs = fs_uber,
    rs = rs,
    uniforms = uniforms_base,
    samplers = samplers_upsample,
}

-- return pass array
return {
    pass_prefilter,
	pass_downsample,
    pass_upsample,
    pass_uber,
}
