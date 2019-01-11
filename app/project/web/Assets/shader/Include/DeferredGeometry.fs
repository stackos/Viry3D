precision highp float;

UniformTexture(0, 1) uniform sampler2D u_texture;

UniformBuffer(0, 2) uniform UniformBuffer02
{
	vec4 u_clip_plane;
} buf_0_2;

Input(0) vec3 v_pos_view;
Input(1) vec3 v_normal_view;
Input(2) vec2 v_uv;

Output(0) vec4 o_color;
Output(1) vec4 o_pos;
Output(2) vec4 o_normal;

float linear_depth(float depth)
{
	float z = depth * 2.0 - 1.0;
    float n = buf_0_2.u_clip_plane.x;
    float f = buf_0_2.u_clip_plane.y;
	return (2.0 * n * f) / (f + n - z * (f - n));
}

void main()
{
    o_color = texture(u_texture, v_uv);
    o_pos.xyz = v_pos_view;
    o_pos.w = linear_depth(gl_FragCoord.z);
    o_normal.xyz = normalize(v_normal_view) * 0.5 + 0.5;
    o_normal.w = 0.0;
}
