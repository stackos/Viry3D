UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
    vec4 u_uv_scale_offset;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec3 a_pos;
Input(2) vec2 a_uv;
Input(4) vec3 a_normal;

Output(0) vec3 v_pos_view;
Output(1) vec3 v_normal_view;
Output(2) vec2 v_uv;

void main()
{
    mat4 model_mat = buf_1_0.u_model_matrix;
    vec4 pos = vec4(a_pos, 1.0);
    mat4 model_view_mat = model_mat * buf_0_0.u_view_matrix;
    vec4 pos_view = pos * model_view_mat;
	gl_Position = pos_view * buf_0_0.u_projection_matrix;

    v_pos_view = pos_view.xyz;
    v_normal_view = normalize((vec4(a_normal, 0.0) * model_view_mat).xyz);
	v_uv = a_uv * buf_0_0.u_uv_scale_offset.xy + buf_0_0.u_uv_scale_offset.zw;

    vulkan_convert();
}
