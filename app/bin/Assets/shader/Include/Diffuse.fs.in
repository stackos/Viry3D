#ifndef CAST_SHADOW
    #define CAST_SHADOW 0
#endif

#ifndef RECIEVE_SHADOW
    #define RECIEVE_SHADOW 0
#endif

#ifndef LIGHTMAP
    #define LIGHTMAP 0
#endif

precision highp float;

#if (CAST_SHADOW == 0)
	UniformTexture(0, 1) uniform sampler2D u_texture;

	UniformBuffer(0, 2) uniform UniformBuffer02
	{
        vec4 u_ambient_color;
        vec4 u_light_pos;
		vec4 u_light_color;
		float u_light_intensity;

    #if (RECIEVE_SHADOW == 1)
        float u_shadow_strength;
        float u_shadow_z_bias;
        float u_shadow_slope_bias;
        float u_shadow_filter_radius;
    #endif
	} buf_0_2;

    #if (RECIEVE_SHADOW == 1)
        UniformTexture(0, 3) uniform highp sampler2D u_shadow_texture;
    #endif

    #if (LIGHTMAP == 1)
        UniformTexture(0, 4) uniform lowp sampler2DArray u_lightmap;

        UniformBuffer(1, 1) uniform UniformBuffer11
        {
            int u_lightmap_index;
        } buf_1_1;
    #endif

	Input(0) vec4 v_uv;
	Input(1) vec3 v_normal;

    #if (RECIEVE_SHADOW == 1)
        Input(2) vec4 v_pos_light_proj;
    #endif
#endif

Output(0) vec4 o_frag;

void main()
{
#if (CAST_SHADOW == 0)
    vec4 c = texture(u_texture, v_uv.xy);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-buf_0_2.u_light_pos.xyz); // directional light
    
    float nl = max(dot(n, l), 0.0);
#if (LIGHTMAP == 1)
    vec4 lm = texture(u_lightmap, vec3(v_uv.zw, float(buf_1_1.u_lightmap_index)));
    vec3 gi_diffuse = c.rgb * lm.rgb * 2.0;
#else
    vec3 gi_diffuse = c.rgb * buf_0_2.u_ambient_color.rgb;
#endif
    vec3 diffuse = c.rgb * nl * buf_0_2.u_light_color.rgb * buf_0_2.u_light_intensity;

    #if (RECIEVE_SHADOW == 1)
		float shadow_z_bias = buf_0_2.u_shadow_z_bias + buf_0_2.u_shadow_slope_bias * tan(acos(nl));
        float shadow = sample_shadow(u_shadow_texture, v_pos_light_proj / v_pos_light_proj.w, buf_0_2.u_shadow_filter_radius, shadow_z_bias) * buf_0_2.u_shadow_strength;
        diffuse = diffuse * (1.0 - shadow);
    #endif

    c.rgb = gi_diffuse + diffuse;
    c.a = 1.0;

    o_frag = c;
#else
    o_frag = vec4(0.0);
#endif
}
