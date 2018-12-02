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

#ifndef COLORSPACE_GAMMA
    #define COLORSPACE_GAMMA 0
#endif

#if (COLORSPACE_GAMMA == 1)
    #define ColorSpaceDielectricSpec vec4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else
    #define ColorSpaceDielectricSpec vec4(0.04, 0.04, 0.04, 1.0 - 0.04)
#endif
#define PI 3.14159265359f
#define INV_PI 0.31830988618f
#define SPECCUBE_LOD_STEPS 6.0

precision highp float;

#if (CAST_SHADOW == 0)
	UniformTexture(0, 1) uniform sampler2D u_texture;

	UniformBuffer(0, 2) uniform UniformBuffer02
	{
        vec4 u_color;
        float u_metallic;
        float u_smoothness;
        float u_occlusion_strength;
        vec4 u_emission_color;
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

    #if (NROMAL_MAP == 1)
        UniformTexture(0, 4) uniform sampler2D u_normal;
    #endif

    UniformTexture(0, 5) uniform sampler2D u_metallic_smoothness;
    UniformTexture(0, 6) uniform sampler2D u_occlusion;
    UniformTexture(0, 7) uniform sampler2D u_emission;
    UniformTexture(0, 8) uniform samplerCube u_environment;

	Input(0) vec4 v_uv;
    Input(1) vec3 v_view_dir;

    #if (RECIEVE_SHADOW == 1)
        Input(2) vec4 v_pos_light_proj;
    #endif

    #if (INSTANCING == 1)
        Input(3) vec4 v_metallic_smoothness;
    #endif

    #if (NROMAL_MAP == 1)
        Input(4) vec4 v_tangent_to_world[3];
    #else
        Input(4) vec3 v_normal;
    #endif

	Output(0) vec4 o_frag;

    struct SurfaceOutput
    {
        vec3 albedo;
        vec3 normal;
        vec3 emission;
        float metallic;
        float smoothness;
        float occlusion;
        float alpha;
    };

    void surface(vec2 uv, inout SurfaceOutput s)
    {
        vec4 albedo = texture(u_texture, uv) * buf_0_2.u_color;
        vec4 metallic_smoothness = texture(u_metallic_smoothness, uv);
        float metallic = metallic_smoothness.r * buf_0_2.u_metallic;
        float smoothness = metallic_smoothness.g * buf_0_2.u_smoothness;
        float occlusion = mix(1.0, texture(u_occlusion, uv).g, buf_0_2.u_occlusion_strength);
        vec4 emission = texture(u_emission, uv) * buf_0_2.u_emission_color;

        #if (INSTANCING == 1)
            metallic *= v_metallic_smoothness.x;
            smoothness *= v_metallic_smoothness.y;
        #endif

        #if (NROMAL_MAP == 1)
            s.normal = texture(u_normal, uv).xyz * 2.0 - 1.0;
        #endif

        s.albedo = albedo.rgb;
        s.emission = emission.rgb;
        s.metallic = metallic;
        s.smoothness = smoothness;
        s.occlusion = occlusion;
        s.alpha = albedo.a;
    }

    float one_minus_reflectivity_from_metallic(float metallic)
    {
        float one_minus_dielectric_spec = ColorSpaceDielectricSpec.a;
        return one_minus_dielectric_spec - metallic * one_minus_dielectric_spec;
    }

    vec3 diffuse_and_specular_from_metallic(vec3 albedo, float metallic, out vec3 spec_color, out float one_minus_reflectivity)
    {
        spec_color = mix(ColorSpaceDielectricSpec.rgb, albedo, metallic);
        one_minus_reflectivity = one_minus_reflectivity_from_metallic(metallic);
        return albedo * one_minus_reflectivity;
    }

    float smoothness_to_perceptual_roughness(float smoothness)
    {
        return (1.0 - smoothness);
    }

    float perceptual_roughness_to_mipmap_level(float perceptual_roughness)
    {
        return perceptual_roughness * SPECCUBE_LOD_STEPS;
    }

    float perceptual_roughness_to_roughness(float perceptual_roughness)
    {
        return perceptual_roughness * perceptual_roughness;
    }

    vec3 gi_diffuse(float occlusion, vec2 lightmap_uv)
    {
        vec3 diffuse;

        diffuse = buf_0_2.u_ambient_color.rgb;
        diffuse *= occlusion;

        return diffuse;
    }

    vec3 glossy_environment(float perceptual_roughness, vec3 refl_uvw)
    {
        perceptual_roughness = perceptual_roughness * (1.7 - 0.7 * perceptual_roughness);

        float mip = perceptual_roughness_to_mipmap_level(perceptual_roughness);
        vec4 env = textureLod(u_environment, refl_uvw, mip);

#if (COLORSPACE_GAMMA == 0)
    return pow(env.rgb, vec3(2.2));
#else
    return env.rgb;
#endif
    }

    vec3 gi_specular(float occlusion, float perceptual_roughness, vec3 refl_uvw)
    {
        vec3 specular;

        specular = glossy_environment(perceptual_roughness, refl_uvw);
        specular = specular * occlusion;

        return specular;
    }

    float pow5(float x)
    {
        return x * x * x * x * x;
    }

    float disney_diffuse(float nv, float nl, float lh, float perceptual_roughness)
    {
        float fd90 = 0.5 + 2.0 * lh * lh * perceptual_roughness;
        float light_scatter = (1.0 + (fd90 - 1.0) * pow5(1.0 - nl));
        float view_scatter = (1.0 + (fd90 - 1.0) * pow5(1.0 - nv));

        return light_scatter * view_scatter;
    }

    float smith_joint_ggx_visibility_term(float nl, float nv, float roughness)
    {
        float a = roughness;
        float v = nl * (nv * (1.0 - a) + a);
        float l = nv * (nl * (1.0 - a) + a);

        return 0.5 / (v + l + 1e-5);
    }

    float ggx_term(float nh, float roughness)
    {
        float a2 = roughness * roughness;
        float d = (nh * a2 - nh) * nh + 1.0;
        return INV_PI * a2 / (d * d + 1e-7);
    }

    vec3 fresnel_term(vec3 f0, float cosa)
    {
        float t = pow5(1.0 - cosa);
        return f0 + (1.0 - f0) * t;
    }

    vec3 fresnel_lerp(vec3 f0, vec3 f90, float cosa)
    {
        float t = pow5(1.0 - cosa);
        return mix(f0, f90, t);
    }

    vec4 brdf_pbs(vec3 diff, vec3 spec, float one_minus_reflectivity, float smoothness, vec3 normal, vec3 view_dir, vec3 light_dir, vec3 light_color, vec3 gi_diff, vec3 gi_spec)
    {
        float perceptual_roughness = smoothness_to_perceptual_roughness(smoothness);
        vec3 h = normalize(light_dir + view_dir);
        float nv = abs(dot(normal, view_dir));
        float nl = clamp(dot(normal, light_dir), 0.0, 1.0);
        float nh = clamp(dot(normal, h), 0.0, 1.0);
        float lv = clamp(dot(light_dir, view_dir), 0.0, 1.0);
        float lh = clamp(dot(light_dir, h), 0.0, 1.0);

        float diffuse_term = disney_diffuse(nv, nl, lh, perceptual_roughness) * nl;

        float roughness = perceptual_roughness_to_roughness(perceptual_roughness);
        roughness = max(roughness, 0.002);
        float v = smith_joint_ggx_visibility_term(nl, nv, roughness);
        float d = ggx_term(nh, roughness);
        float spec_term = v * d * PI;

#if (COLORSPACE_GAMMA == 1)
        spec_term = sqrt(max(1e-4, spec_term));
#endif
        spec_term = max(0.0, spec_term * nl);
        if (spec.r > 0.0 || spec.g > 0.0 || spec.b > 0.0)
        {
            spec_term *= 1.0;
        }
        else
        {
            spec_term = 0.0;
        }

#if (COLORSPACE_GAMMA == 1)
        float surface_reduction = 1.0 - 0.28 * roughness * perceptual_roughness;
#else
        float surface_reduction = 1.0 / (roughness * roughness + 1.0);
#endif
        float grazing_term = clamp(smoothness + (1.0 - one_minus_reflectivity), 0.0, 1.0);

        vec3 color = diff * (gi_diff + light_color * diffuse_term)
            + spec_term * light_color * fresnel_term(spec, lh)
            + surface_reduction * gi_spec * fresnel_lerp(spec, vec3(grazing_term), nv);
        
        return vec4(color, 1.0);
    }

    vec4 lighting(SurfaceOutput s, vec3 view_dir, vec3 light_dir, vec3 light_color, vec2 lightmap_uv)
    {
        float one_minus_reflectivity;
        vec3 spec_color;
        s.albedo = diffuse_and_specular_from_metallic(s.albedo, s.metallic, spec_color, one_minus_reflectivity);

        float perceptual_roughness = smoothness_to_perceptual_roughness(s.smoothness);
        vec3 refl_uvw = reflect(-view_dir, s.normal);

        vec3 gi_diff = gi_diffuse(s.occlusion, lightmap_uv);
        vec3 gi_spec = gi_specular(s.occlusion, perceptual_roughness, refl_uvw);

        vec4 c = brdf_pbs(s.albedo, spec_color, one_minus_reflectivity, s.smoothness, s.normal, view_dir, light_dir, light_color, gi_diff, gi_spec);
        c.rgb += s.emission;
        c.a = s.alpha;

        return c;
    }
#endif

void main()
{
#if (CAST_SHADOW == 0)
    SurfaceOutput s;
    surface(v_uv.xy, s);

    #if (NROMAL_MAP == 1)
        vec3 tangent = v_tangent_to_world[0].xyz;
        vec3 binormal = v_tangent_to_world[1].xyz;
        vec3 normal = v_tangent_to_world[2].xyz;
        vec3 pos_world = vec3(v_tangent_to_world[0].w, v_tangent_to_world[1].w, v_tangent_to_world[2].w);
        vec3 normal_tangent = normalize(s.normal);
        s.normal = normalize(tangent * normal_tangent.x + binormal * normal_tangent.y + normal * normal_tangent.z);
    #else
        s.normal = normalize(v_normal);
    #endif

    vec3 view_dir = normalize(v_view_dir);
    vec3 light_dir = normalize(-buf_0_2.u_light_pos.xyz); // directional light
    vec3 light_color = buf_0_2.u_light_color.rgb * buf_0_2.u_light_intensity;

    vec4 c = lighting(s, view_dir, light_dir, light_color, v_uv.zw);

    #if (RECIEVE_SHADOW == 1)
        float nl = max(dot(s.normal, light_dir), 0.0);
		float shadow_z_bias = buf_0_2.u_shadow_z_bias + buf_0_2.u_shadow_slope_bias * tan(acos(nl));
        float shadow = sample_shadow(u_shadow_texture, v_pos_light_proj / v_pos_light_proj.w, buf_0_2.u_shadow_filter_radius, shadow_z_bias) * buf_0_2.u_shadow_strength;
        c.rgb = c.rgb * (1.0 - shadow);
    #endif

    #if (COLORSPACE_GAMMA == 0)
        o_frag = pow(c, vec4(1.0 / 2.2));
    #else
        o_frag = c;
    #endif
#endif
}
