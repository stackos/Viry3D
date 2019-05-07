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

#ifndef NROMAL_MAP
    #define NROMAL_MAP 0
#endif

UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;

#if (CAST_SHADOW == 0)
    vec4 u_uv_scale_offset;
    vec4 u_camera_pos;

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
    } buf_1_0;
#endif

Input(0) vec3 a_pos;

#if (CAST_SHADOW == 0)
	Input(2) vec2 a_uv;
    Input(4) vec3 a_normal;
    Input(5) vec4 a_tangent;

	Output(0) vec4 v_uv;
    Output(1) vec3 v_view_dir;

    #if (RECIEVE_SHADOW == 1)
        Output(2) vec4 v_pos_light_proj;
    #endif

    #if (INSTANCING == 1)
        Output(3) vec4 v_metallic_smoothness;
    #endif

    #if (NROMAL_MAP == 1)
        Output(4) vec4 v_tangent_to_world[3];
    #else
        Output(4) vec3 v_normal;
    #endif
#endif

#if (INSTANCING == 1)
    Input(8) vec4 a_instance_matrix_row_0;
    Input(9) vec4 a_instance_matrix_row_1;
    Input(10) vec4 a_instance_matrix_row_2;
    Input(11) vec4 a_instance_matrix_row_3;
    Input(12) vec4 a_instance_metallic_smoothness;
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
    v_metallic_smoothness = a_instance_metallic_smoothness;
#endif

    vec4 pos_world = vec4(a_pos, 1.0) * model_mat;

	gl_Position = pos_world * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix;

#if (CAST_SHADOW == 0)
	v_uv.xy = a_uv * buf_0_0.u_uv_scale_offset.xy + buf_0_0.u_uv_scale_offset.zw;
    
    vec3 normal_world = normalize((vec4(a_normal, 0) * model_mat).xyz);

    #if (NROMAL_MAP == 1)
        vec3 tangent_world = normalize((vec4(a_tangent.xyz, 0) * model_mat).xyz);
        vec3 binormal_world = cross(normal_world, tangent_world) * a_tangent.w;

        v_tangent_to_world[0].xyz = tangent_world;
        v_tangent_to_world[1].xyz = binormal_world;
        v_tangent_to_world[2].xyz = normal_world;
        v_tangent_to_world[0].w = pos_world.x;
        v_tangent_to_world[1].w = pos_world.y;
        v_tangent_to_world[2].w = pos_world.z;
    #else
        v_normal = normal_world;
    #endif

    #if (RECIEVE_SHADOW == 1)
        v_pos_light_proj = pos_world * buf_0_0.u_light_view_projection_matrix;
    #endif

    v_view_dir = buf_0_0.u_camera_pos.xyz - pos_world.xyz;
    v_uv.zw = vec2(0.0);
#endif
	
    vulkan_convert();
}
