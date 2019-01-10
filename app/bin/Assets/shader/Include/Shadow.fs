precision highp float;

#ifndef VERSION_100_ES
	#define VERSION_100_ES 0
#endif

#if VERSION_100_ES
vec2 Poisson25[25];
#else
const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);
#endif

float texture_shadow(highp sampler2D shadow_texture, vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        return 1.0;
    }
    else
    {
#if VERSION_100_ES
		return texture2D(shadow_texture, vec2(uv.x, 1.0 - uv.y)).r;
#else
		return texture(shadow_texture, uv).r;
#endif
    }
}

float poisson_filter(highp sampler2D shadow_texture, float z, vec2 uv, float shadow_z_bias, vec2 filter_radius)
{
    float shadow = 0.0;
    for (int i = 0; i < 25; ++i)
    {
        vec2 offset = Poisson25[i] * filter_radius;
        float shadow_depth = texture_shadow(shadow_texture, uv + offset);
        if (z - shadow_z_bias > shadow_depth)
        {
            shadow += 1.0;
        }
    }
    return shadow / 25.0;
}

float pcf_filter(highp sampler2D shadow_texture, float z, vec2 uv, float shadow_z_bias, vec2 filter_radius)
{
    float shadow = 0.0;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            vec2 offset = vec2(i, j) * filter_radius;
            float shadow_depth = texture_shadow(shadow_texture, uv + offset);
            if (z - shadow_z_bias > shadow_depth)
            {
                shadow += 1.0;
            }
        }
    }
    return shadow / 9.0;
}

float linear_filter(highp sampler2D shadow_texture, float z, vec2 uv, float shadow_z_bias)
{
    float shadow_depth = texture_shadow(shadow_texture, uv);
    if (z - shadow_z_bias > shadow_depth)
    {
        return 1.0;
    }
	else
	{
		return 0.0;
	}
}

float sample_shadow(highp sampler2D shadow_texture, vec4 pos_light_proj, float shadow_filter_radius, float shadow_z_bias)
{
    vec2 uv = pos_light_proj.xy * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;
    float z = pos_light_proj.z * 0.5 + 0.5;
    vec2 filter_radius = vec2(shadow_filter_radius);

#if VERSION_100_ES
    // TODO:
    // use texture lookup intead poisson table
    Poisson25[0] = vec2(-0.978698, -0.0884121);
    Poisson25[1] = vec2(-0.841121, 0.521165);
    Poisson25[2] = vec2(-0.71746, -0.50322);
    Poisson25[3] = vec2(-0.702933, 0.903134);
    Poisson25[4] = vec2(-0.663198, 0.15482);
    Poisson25[5] = vec2(-0.495102, -0.232887);
    Poisson25[6] = vec2(-0.364238, -0.961791);
    Poisson25[7] = vec2(-0.345866, -0.564379);
    Poisson25[8] = vec2(-0.325663, 0.64037);
    Poisson25[9] = vec2(-0.182714, 0.321329);
    Poisson25[10] = vec2(-0.142613, -0.0227363);
    Poisson25[11] = vec2(-0.0564287, -0.36729);
    Poisson25[12] = vec2(-0.0185858, 0.918882);
    Poisson25[13] = vec2(0.0381787, -0.728996);
    Poisson25[14] = vec2(0.16599, 0.093112);
    Poisson25[15] = vec2(0.253639, 0.719535);
    Poisson25[16] = vec2(0.369549, -0.655019);
    Poisson25[17] = vec2(0.423627, 0.429975);
    Poisson25[18] = vec2(0.530747, -0.364971);
    Poisson25[19] = vec2(0.566027, -0.940489);
    Poisson25[20] = vec2(0.639332, 0.0284127);
    Poisson25[21] = vec2(0.652089, 0.669668);
    Poisson25[22] = vec2(0.773797, 0.345012);
    Poisson25[23] = vec2(0.968871, 0.840449);
    Poisson25[24] = vec2(0.991882, -0.657338);
#endif

    return poisson_filter(shadow_texture, z, uv, shadow_z_bias, filter_radius);

    //return pcf_filter(shadow_texture, z, uv, shadow_z_bias, filter_radius);
    //return linear_filter(shadow_texture, z, uv, shadow_z_bias);
}
