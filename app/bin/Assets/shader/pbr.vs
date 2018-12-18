UniformBuffer(0, 0) uniform Material {
    mat4 viewProjection;
    mat4 reflectionMatrix;
    vec3 vSphericalX;
    vec3 vSphericalY;
    vec3 vSphericalZ;
    vec3 vSphericalXX_ZZ;
    vec3 vSphericalYY_ZZ;
    vec3 vSphericalZZ;
    vec3 vSphericalXY;
    vec3 vSphericalYZ;
    vec3 vSphericalZX;
};
UniformBuffer(1, 0) uniform Scene {
    mat4 world;
};
Input(0) vec3 position;
Input(1) vec3 normal;
Input(2) vec2 uv;
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
    vec3 positionUpdated = position;
    vec3 normalUpdated = normal;
    mat4 finalWorld = world;
    gl_Position = viewProjection * finalWorld * vec4(positionUpdated, 1.0);
    vec4 worldPos = finalWorld * vec4(positionUpdated, 1.0);
    vPositionW = vec3(worldPos);
    mat3 normalWorld = mat3(finalWorld);
    vNormalW = normalize(normalWorld * normalUpdated);
    vec3 reflectionVector = vec3(reflectionMatrix * vec4(vNormalW, 0)).xyz;
    vEnvironmentIrradiance = environmentIrradianceJones(reflectionVector);
    vec2 uv2 = vec2(0., 0.);
    vMainUV1 = uv;
}