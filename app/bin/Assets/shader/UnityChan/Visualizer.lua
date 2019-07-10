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

VK_LAYOUT_LOCATION(0) out vec4 v_pos_proj;
VK_LAYOUT_LOCATION(1) out vec4 v_pos_world;
VK_LAYOUT_LOCATION(2) out vec4 v_time;
VK_LAYOUT_LOCATION(3) out mat4 v_view_projection_matrix;

void main()
{
    mat4 model_matrix = u_model_matrix;
    vec4 pos_world = i_vertex * model_matrix;
	gl_Position = pos_world * u_view_matrix * u_projection_matrix;
    v_pos_proj = gl_Position;
    v_pos_world = pos_world;
	v_time = u_time;
	v_view_projection_matrix = u_view_matrix * u_projection_matrix;

	vk_convert();
}
]]

local fs = [[
#ifndef VR_GLES
#define VR_GLES 0
#endif

precision highp float;
VK_SAMPLER_BINDING(0) uniform sampler2D u_reflection_texture;
VK_SAMPLER_BINDING(1) uniform highp sampler2D u_reflection_depth_texture;
VK_UNIFORM_BINDING(4) uniform PerMaterialFragment
{
	vec4 _Spectra;
	vec4 _Center;
	vec4 _RingParams;
	vec4 _RingSpeed;
	vec4 _GridColor;
    vec4 _ReflectionStrength;
};

VK_LAYOUT_LOCATION(0) in vec4 v_pos_proj;
VK_LAYOUT_LOCATION(1) in vec4 v_pos_world;
VK_LAYOUT_LOCATION(2) in vec4 v_time;
VK_LAYOUT_LOCATION(3) in mat4 v_view_projection_matrix;

layout(location = 0) out vec4 o_color;

float _gl_mod(float a, float b) { return a - b * floor(a/b); }
vec2 _gl_mod(vec2 a, vec2 b) { return a - b * floor(a/b); }
vec3 _gl_mod(vec3 a, vec3 b) { return a - b * floor(a/b); }
vec4 _gl_mod(vec4 a, vec4 b) { return a - b * floor(a/b); }

float iq_rand(float p)
{
	return fract(sin(p) * 43758.5453);
}

float rings(vec3 pos)
{
	float pi = 3.14159;
	vec2 wpos = pos.xz;
	float stride = _RingParams.x;
	float strine_half = stride * 0.5;
	float thickness = 1.0 - (_RingParams.y + length(_Spectra) * (_RingParams.z - _RingParams.y));
	float distance = abs(length(wpos) - v_time.y * 0.1);
	float fra = _gl_mod(distance, stride);
	float cycle = floor(distance / stride);
	float c = strine_half - abs(fra - strine_half) - strine_half * thickness;
	c = max(c * (1.0 / (strine_half * thickness)), 0.0);
	float rs = iq_rand(cycle * cycle);
	float r = iq_rand(cycle) + v_time.y * (_RingSpeed.x + (_RingSpeed.y - _RingSpeed.x) * rs);
	float angle = atan(wpos.y, wpos.x) / pi * 0.5 + 0.5;
	float a = 1.0 - _gl_mod(angle + r, 1.0);
	a = max(a - 0.7, 0.0) * c;
    return a;
}

float hex(vec2 p, vec2 h)
{
	vec2 q = abs(p);
	return max(q.x - h.y, max(q.x + q.y * 0.57735, q.y * 1.1547) - h.x);
}

float hex_grid(vec3 p)
{
	float scale = 1.2;
	vec2 grid = vec2(0.692, 0.4) * scale;
	float radius = 0.22 * scale;
	vec2 p1 = _gl_mod(p.xz, grid) - grid * 0.5;
	float c1 = hex(p1, vec2(radius, radius));
	vec2 p2 = _gl_mod(p.xz + grid * 0.5, grid) - grid * 0.5;
	float c2 = hex(p2, vec2(radius, radius));
	return min(c1, c2);
}

vec3 guess_normal(vec3 p)
{
	const float d = 0.01;
	return normalize(vec3(
		hex_grid(p + vec3(d, 0.0, 0.0)) - hex_grid(p + vec3(-d, 0.0, 0.0)),
		hex_grid(p + vec3(0.0, d, 0.0)) - hex_grid(p + vec3(0.0, -d, 0.0)),
		hex_grid(p + vec3(0.0, 0.0, d)) - hex_grid(p + vec3(0.0, 0.0, -d))
		));
}

float circle(vec3 pos)
{
	float o_radius = 5.0;
	float i_radius = 4.0;
	float d = length(pos.xz);
	float c = max(o_radius - (o_radius - _gl_mod(d - v_time.y * 1.5, o_radius)) - i_radius, 0.0);
	return c;
}

vec2 flip_uv_y(vec2 uv)
{
#if (VR_GLES == 1)
    return vec2(uv.x, 1.0 - uv.y);
#else
	return uv;
#endif
}

const float blur_radius = 0.005;
const vec2 blur_coords[9] = vec2[](
    vec2(0.000, 0.000),
    vec2(0.1080925165271518, -0.9546740999616308) * blur_radius,
    vec2(-0.4753686437884934, -0.8417212473681748) * blur_radius,
    vec2(0.7242715177221273, -0.6574584801064549) * blur_radius,
    vec2(-0.023355087558461607, 0.7964400038854089) * blur_radius,
    vec2(-0.8308210026544296, -0.7015103725420933) * blur_radius,
    vec2(0.3243705688309195, 0.2577797517167695) * blur_radius,
    vec2(0.31851240326305463, -0.2220789454739755) * blur_radius,
    vec2(-0.36307729185097637, -0.7307245945773899) * blur_radius
);

void main()
{
    vec3 center = v_pos_world.xyz - _Center.xyz;
    float trails = rings(center);
	float grid_d = hex_grid(center);
	float grid = grid_d > 0.0 ? 1.0 : 0.0;
	vec3 n = guess_normal(center);
	n = (vec4(n, 0.0) * v_view_projection_matrix).xyz;

	vec4 c = vec4(0.0);
    c += trails * (0.5 + _Spectra * _RingParams.w);
	c += _GridColor * (grid * circle(center)) * _GridColor.w;

	// reflection
    float depth = 1.0;
    vec2 coord = v_pos_proj.xy / v_pos_proj.w * 0.5 + 0.5;
    depth = texture(u_reflection_depth_texture, flip_uv_y(coord)).r;
    for (int i = 1; i < 9; ++i)
    {
        depth = min(depth, texture(u_reflection_depth_texture, flip_uv_y(coord + blur_coords[i])).r);
    }

    float refpos = coord.y * 2.0 - 1.0;

    float fade_by_depth = 1.0;
    fade_by_depth = max(1.0 - abs(refpos) * 0.3, 0.0);
    vec3 refcolor = vec3(0.0);

    float g = clamp((grid_d + 0.02) * 50.0, 0.0, 1.0);
    coord += n.xz * (g > 0.0 && g < 1.0 ? 1.0 : 0.0) * 0.02;
    for (int i = 0; i < 9; ++i)
    {
        refcolor += texture(u_reflection_texture, flip_uv_y(coord + blur_coords[i] * ((1.0 - fade_by_depth) * 0.75 + 0.25))).rgb * 0.1111;
    }

    c.rgb += refcolor * _ReflectionStrength.x * fade_by_depth * (1.0 - grid * 0.9);

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
                    name = "_Spectra",
                    size = 16,
                },
				{
                    name = "_Center",
                    size = 16,
                },
				{
                    name = "_RingParams",
                    size = 16,
                },
				{
                    name = "_RingSpeed",
                    size = 16,
                },
				{
                    name = "_GridColor",
                    size = 16,
                },
                {
                    name = "_ReflectionStrength",
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
                    name = "u_reflection_texture",
                    binding = 0,
                },
                {
                    name = "u_reflection_depth_texture",
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
