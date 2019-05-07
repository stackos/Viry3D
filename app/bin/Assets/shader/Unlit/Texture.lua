vs = [[
	VK_UNIFORM_BINDING(0) uniform PerView
	{
		mat4 u_vp;
	};
	VK_UNIFORM_BINDING(1) uniform PerRenderer
	{
		mat4 u_world;
	};
	layout(location = 0) in vec4 i_position;
	layout(location = 1) in vec2 i_uv;
	VK_LAYOUT_LOCATION(0) out vec2 v_uv;
	void main()
	{
		gl_Position = i_position * u_world * u_vp;
		v_uv = i_uv;

		vk_convert();
	}
]]

fs = [[
	precision highp float;
	VK_SAMPLER_BINDING(0) uniform sampler2D u_texture;
	VK_LAYOUT_LOCATION(0) in vec2 v_uv;
	layout(location = 0) out vec4 o_color;
	void main()
	{
		o_color = texture(u_texture, v_uv);
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
	    One Zero SrcColor SrcAlpha DstColor DstAlpha OneMinusSrcColor OneMinusSrcAlpha OneMinusDstColor OneMinusDstAlpha
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
}

-- return pass array
return {
    pass
}
