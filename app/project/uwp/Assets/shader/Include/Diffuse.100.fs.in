#ifndef CAST_SHADOW
    #define CAST_SHADOW 0
#endif

#ifndef RECIEVE_SHADOW
    #define RECIEVE_SHADOW 0
#endif

precision highp float;

#if (CAST_SHADOW == 0)
	uniform sampler2D u_texture;

    uniform vec4 u_ambient_color;
    uniform vec4 u_light_pos;
	uniform vec4 u_light_color;
	uniform float u_light_intensity;

    #if (RECIEVE_SHADOW == 1)
        uniform float u_shadow_strength;
        uniform float u_shadow_z_bias;
        uniform float u_shadow_slope_bias;
        uniform float u_shadow_filter_radius;
    #endif

    #if (RECIEVE_SHADOW == 1)
        uniform highp sampler2D u_shadow_texture;
    #endif

	varying vec2 v_uv;
	varying vec3 v_normal;

    #if (RECIEVE_SHADOW == 1)
        varying vec4 v_pos_light_proj;
    #endif
#endif

void main()
{
#if (CAST_SHADOW == 0)
    vec4 c = texture2D(u_texture, v_uv);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-u_light_pos.xyz); // directional light
    
    float nl = max(dot(n, l), 0.0);
    vec3 ambient = c.rgb * u_ambient_color.rgb;
    vec3 diffuse = c.rgb * nl * u_light_color.rgb * u_light_intensity;

    #if (RECIEVE_SHADOW == 1)
		float shadow_z_bias = u_shadow_z_bias + u_shadow_slope_bias * tan(acos(nl));
        float shadow = sample_shadow(u_shadow_texture, v_pos_light_proj / v_pos_light_proj.w, u_shadow_filter_radius, shadow_z_bias) * u_shadow_strength;
        diffuse = diffuse * (1.0 - shadow);
    #endif

    c.rgb = ambient + diffuse;
    c.a = 1.0;

    gl_FragColor = c;
#endif
}
