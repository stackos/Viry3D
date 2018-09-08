#ifndef SKINNED_MESH
    #define SKINNED_MESH 0
#endif

#ifndef CAST_SHADOW
    #define CAST_SHADOW 0
#endif

#ifndef RECIEVE_SHADOW
    #define RECIEVE_SHADOW 0
#endif

uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
#if (CAST_SHADOW == 0)
    uniform vec4 u_uv_scale_offset;

    #if (RECIEVE_SHADOW == 1)
        uniform mat4 u_light_view_projection_matrix;
    #endif
#endif

#if (SKINNED_MESH == 1)
    #define BONE_VECTOR_MAX 90

	uniform vec4 u_bones[BONE_VECTOR_MAX];

    attribute vec4 a_bone_weights;
    attribute vec4 a_bone_indices;
#else
    uniform mat4 u_model_matrix;
#endif

attribute vec4 a_pos;

#if (CAST_SHADOW == 0)
	attribute vec2 a_uv;
	attribute vec3 a_normal;

	varying vec2 v_uv;
	varying vec3 v_normal;

    #if (RECIEVE_SHADOW == 1)
        varying vec4 v_pos_light_proj;
    #endif
#endif

void main()
{
#if (SKINNED_MESH == 1)
    mat4 model_mat;
    {
        int index_0 = int(a_bone_indices.x);
        int index_1 = int(a_bone_indices.y);
        int index_2 = int(a_bone_indices.z);
        int index_3 = int(a_bone_indices.w);

        float weights_0 = a_bone_weights.x;
        float weights_1 = a_bone_weights.y;
        float weights_2 = a_bone_weights.z;
        float weights_3 = a_bone_weights.w;

	    mat4 bone_0 = mat4(u_bones[index_0*3], u_bones[index_0*3+1], u_bones[index_0*3+2], vec4(0, 0, 0, 1));
	    mat4 bone_1 = mat4(u_bones[index_1*3], u_bones[index_1*3+1], u_bones[index_1*3+2], vec4(0, 0, 0, 1));
	    mat4 bone_2 = mat4(u_bones[index_2*3], u_bones[index_2*3+1], u_bones[index_2*3+2], vec4(0, 0, 0, 1));
	    mat4 bone_3 = mat4(u_bones[index_3*3], u_bones[index_3*3+1], u_bones[index_3*3+2], vec4(0, 0, 0, 1));
	    model_mat = bone_0 * weights_0 + bone_1 * weights_1 + bone_2 * weights_2 + bone_3 * weights_3;
    }
#else
    mat4 model_mat = u_model_matrix;
#endif

	gl_Position = a_pos * model_mat * u_view_matrix * u_projection_matrix;

#if (CAST_SHADOW == 0)
	v_uv = a_uv * u_uv_scale_offset.xy + u_uv_scale_offset.zw;
    v_normal = normalize((vec4(a_normal, 0) * model_mat).xyz);

    #if (RECIEVE_SHADOW == 1)
        v_pos_light_proj = a_pos * model_mat * u_light_view_projection_matrix;
    #endif
#endif
}
