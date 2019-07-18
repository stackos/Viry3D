Shader "Diffuse" {
Properties {
    u_texture("Texture", 2D) = "white" {}
    u_uv_scale_offset("UV Scale Offset", Vector) = (1, 1, 0, 0)
    u_color("Color", Color) = (1, 1, 1, 1)
    _Color("Main Color", Color) = (1, 1, 1, 1)
}
SubShader {
    CGPROGRAM
    #pragma surface surf Lambert

    sampler2D u_texture;
    float4 u_uv_scale_offset;
    float4 u_color;
    float4 _Color;
    
    struct Input {
        float2 uvu_texture;
    };

    void surf (Input IN, inout SurfaceOutput o) {
        float4 c = tex2D(u_texture, IN.uvu_texture * u_uv_scale_offset.xy + u_uv_scale_offset.zw);
        o.Albedo = c.rgb * u_color.rgb;
        o.Alpha = c.a * u_color.a;
    }
    ENDCG
}
}
