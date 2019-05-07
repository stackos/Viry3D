#ifndef SKINNED_MESH
    #define SKINNED_MESH 0
#endif

#ifndef CAST_SHADOW
    #define CAST_SHADOW 0
#endif

#ifndef RECIEVE_SHADOW
    #define RECIEVE_SHADOW 0
#endif

#ifndef INSTANCING
    #define INSTANCING 0
#endif

#ifndef LIGHTMAP
    #define LIGHTMAP 0
#endif

#ifndef MULTIVIEW
    #define MULTIVIEW 0
#endif

#if (MULTIVIEW == 1)
#extension GL_EXT_multiview : enable
#endif

UniformBuffer(0, 0) uniform UniformBuffer00
{
#if (MULTIVIEW == 1)
    mat4 u_view_matrix[2];
#else
    mat4 u_view_matrix;
#endif
	mat4 u_projection_matrix;

#if (CAST_SHADOW == 0)
    vec4 u_uv_scale_offset;

    #if (RECIEVE_SHADOW == 1)
        mat4 u_light_view_projection_matrix;
    #endif
#endif
} buf_0_0;

#if (SKINNED_MESH == 1)
    #define BONE_VECTOR_MAX 210

    UniformBuffer(1, 0) uniform UniformBuffer10
    {
	    vec4 u_bones[BONE_VECTOR_MAX];
    } buf_1_0;

    Input(6) vec4 a_bone_weights;
    Input(7) vec4 a_bone_indices;
#else
    UniformBuffer(1, 0) uniform UniformBuffer10
    {
	    mat4 u_model_matrix;
    #if (LIGHTMAP == 1)
        vec4 u_lightmap_scale_offset;
    #endif
    } buf_1_0;
#endif

Input(0) vec3 a_pos;

#if (CAST_SHADOW == 0)
	Input(2) vec2 a_uv;
	Input(4) vec3 a_normal;

    #if (LIGHTMAP == 1)
        Input(3) vec2 a_uv2;
    #endif

	Output(0) vec4 v_uv;
	Output(1) vec3 v_normal;

    #if (RECIEVE_SHADOW == 1)
        Output(2) vec4 v_pos_light_proj;
    #endif
#endif

#if (INSTANCING == 1)
    Input(8) vec4 a_instance_matrix_row_0;
    Input(9) vec4 a_instance_matrix_row_1;
    Input(10) vec4 a_instance_matrix_row_2;
    Input(11) vec4 a_instance_matrix_row_3;
#endif

void main()
{
#if (SKINNED_MESH == 1)
    mat4 model_mat;
    SKIN_MAT(model_mat, a_bone_weights, a_bone_indices, buf_1_0.u_bones);
#else
    mat4 model_mat = buf_1_0.u_model_matrix;
#endif

#if (INSTANCING == 1)
    mat4 instance_mat = mat4(a_instance_matrix_row_0, a_instance_matrix_row_1, a_instance_matrix_row_2, a_instance_matrix_row_3);
    model_mat = model_mat * instance_mat;
#endif

#if (MULTIVIEW == 1)
    mat4 view_matrix = buf_0_0.u_view_matrix[gl_ViewIndex];
#else
    mat4 view_matrix = buf_0_0.u_view_matrix;
#endif

    vec4 pos = vec4(a_pos, 1.0);

	gl_Position = pos * model_mat * view_matrix * buf_0_0.u_projection_matrix;

#if (CAST_SHADOW == 0)
	v_uv.xy = a_uv * buf_0_0.u_uv_scale_offset.xy + buf_0_0.u_uv_scale_offset.zw;
    v_normal = normalize((vec4(a_normal, 0) * model_mat).xyz);

    #if (RECIEVE_SHADOW == 1)
        v_pos_light_proj = pos * model_mat * buf_0_0.u_light_view_projection_matrix;
    #endif

    #if (SKINNED_MESH == 0 && LIGHTMAP == 1)
        vec2 uv2 = a_uv2;
        uv2.y = 1.0 - uv2.y;
        uv2 = uv2 * buf_1_0.u_lightmap_scale_offset.xy + buf_1_0.u_lightmap_scale_offset.zw;
        uv2.y = 1.0 - uv2.y;
        v_uv.zw = uv2;
    #else
        v_uv.zw = vec2(0.0);
    #endif
#endif
	
    vulkan_convert();
}
