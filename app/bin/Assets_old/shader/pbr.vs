UniformBuffer(0, 0) uniform Material {
    mat4 u_view_matrix;
	mat4 u_projection_matrix;
    mat4 reflectionMatrixVS;
    vec4 vSphericalX;
    vec4 vSphericalY;
    vec4 vSphericalZ;
    vec4 vSphericalXX_ZZ;
    vec4 vSphericalYY_ZZ;
    vec4 vSphericalZZ;
    vec4 vSphericalXY;
    vec4 vSphericalYZ;
    vec4 vSphericalZX;
};
UniformBuffer(1, 0) uniform Scene {
    mat4 u_model_matrix;
};
Input(0) vec3 a_pos;
Input(4) vec3 a_normal;
Input(2) vec2 a_uv;
Output(0) vec2 vMainUV1;
Output(1) vec3 vPositionW;
Output(2) vec3 vNormalW;
Output(3) vec3 vEnvironmentIrradiance;
vec3 environmentIrradianceJones(vec3 normal) {
    float Nx = normal.x;
    float Ny = normal.y;
    float Nz = normal.z;
    vec3 C1 = vSphericalZZ.rgb;
    vec3 Cx = vSphericalX.rgb;
    vec3 Cy = vSphericalY.rgb;
    vec3 Cz = vSphericalZ.rgb;
    vec3 Cxx_zz = vSphericalXX_ZZ.rgb;
    vec3 Cyy_zz = vSphericalYY_ZZ.rgb;
    vec3 Cxy = vSphericalXY.rgb;
    vec3 Cyz = vSphericalYZ.rgb;
    vec3 Czx = vSphericalZX.rgb;
    vec3 a1 = Cyy_zz * Ny + Cy;
    vec3 a2 = Cyz * Nz + a1;
    vec3 b1 = Czx * Nz + Cx;
    vec3 b2 = Cxy * Ny + b1;
    vec3 b3 = Cxx_zz * Nx + b2;
    vec3 t1 = Cz * Nz + C1;
    vec3 t2 = a2 * Ny + t1;
    vec3 t3 = b3 * Nx + t2;
    return t3;
}
void main(void) {
    vec4 worldPos = vec4(a_pos, 1.0) * u_model_matrix;
    gl_Position = worldPos * u_view_matrix * u_projection_matrix;
    vPositionW = vec3(worldPos);
    mat3 normalWorld = mat3(u_model_matrix);
    vNormalW = normalize(a_normal * normalWorld);
    vec3 reflectionVector = vec3(vec4(vNormalW, 0) * reflectionMatrixVS).xyz;
    vEnvironmentIrradiance = environmentIrradianceJones(reflectionVector);
    vMainUV1 = a_uv;

    vulkan_convert();
}