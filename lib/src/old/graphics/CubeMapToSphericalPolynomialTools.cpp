/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "CubeMapToSphericalPolynomialTools.h"
#include "math/Mathf.h"
#include <assert.h>

namespace Viry3D
{
    SphericalHarmonics::SphericalHarmonics():
        l00(Vector3::Zero()),
        l1_1(Vector3::Zero()),
        l10(Vector3::Zero()),
        l11(Vector3::Zero()),
        l2_2(Vector3::Zero()),
        l2_1(Vector3::Zero()),
        l20(Vector3::Zero()),
        l21(Vector3::Zero()),
        lL22(Vector3::Zero())
    {
    }

    void SphericalHarmonics::AddLight(const Vector3& direction, const Color& color, float delta_soid_angle)
    {
        Vector3 c = Vector3(color.r, color.g, color.b) * delta_soid_angle;

        l00 += c * 0.282095f;

        l1_1 += c * (0.488603f * direction.y);
        l10 += c * (0.488603f * direction.z);
        l11 += c * (0.488603f * direction.x);

        l2_2 += c * (1.092548f * direction.x * direction.y);
        l2_1 += c * (1.092548f * direction.y * direction.z);
        l21 += c * (1.092548f * direction.x * direction.z);

        l20 += c * (0.315392f * (3.f * direction.z * direction.z - 1.f));
        lL22 += c * (0.546274f * (direction.x * direction.x - direction.y * direction.y));
    }

    void SphericalHarmonics::Scale(float scale)
    {
        l00 *= scale;
        l1_1 *= scale;
        l10 *= scale;
        l11 *= scale;
        l2_2 *= scale;
        l2_1 *= scale;
        l20 *= scale;
        l21 *= scale;
        lL22 *= scale;
    }

    void SphericalHarmonics::ConvertIncidentRadianceToIrradiance()
    {
        l00 *= 3.141593f;

        l1_1 *= 2.094395f;
        l10 *= 2.094395f;
        l11 *= 2.094395f;

        l2_2 *= 0.785398f;
        l2_1 *= 0.785398f;
        l20 *= 0.785398f;
        l21 *= 0.785398f;
        lL22 *= 0.785398f;
    }

    void SphericalHarmonics::ConvertIrradianceToLambertianRadiance()
    {
        this->Scale(1.f / Mathf::PI);
    }
    
    SphericalPolynomial::SphericalPolynomial():
        x(Vector3::Zero()),
        y(Vector3::Zero()),
        z(Vector3::Zero()),
        xx(Vector3::Zero()),
        yy(Vector3::Zero()),
        zz(Vector3::Zero()),
        xy(Vector3::Zero()),
        yz(Vector3::Zero()),
        zx(Vector3::Zero())
    {
    }

    void SphericalPolynomial::Scale(float scale)
    {
        x *= scale;
        y *= scale;
        z *= scale;
        xx *= scale;
        yy *= scale;
        zz *= scale;
        yz *= scale;
        zx *= scale;
        xy *= scale;
    }

    SphericalPolynomial SphericalPolynomial::FromHarmonics(const SphericalHarmonics& sh)
    {
        SphericalPolynomial sp;

        sp.x = sh.l11 * 1.02333f;
        sp.y = sh.l1_1 * 1.02333f;
        sp.z = sh.l10 * 1.02333f;

        sp.xx = sh.l00 * 0.886277f - sh.l20 * 0.247708f + sh.lL22 * 0.429043f;
        sp.yy = sh.l00 * 0.886277f - sh.l20 * 0.247708f - sh.lL22 * 0.429043f;
        sp.zz = sh.l00 * 0.886277f + sh.l20 * 0.495417f;

        sp.yz = sh.l2_1 * 0.858086f;
        sp.zx = sh.l21 * 0.858086f;
        sp.xy = sh.l2_2 * 0.858086f;

        sp.Scale(1.f / Mathf::PI);

        return sp;
    }

    SphericalPolynomial CubeMapToSphericalPolynomialTools::ConvertCubeMapToSphericalPolynomial(int size, TextureFormat format, const Vector<ByteBuffer>& faces, bool gamma_space)
    {
        SphericalHarmonics sh;
        float total_solid_angle = 0.f;
        float du = 2.f / static_cast<float>(size);
        float dv = du;
        float min_uv = du * 0.5f - 1.f;
        Vector<Vector3> file_normal = {
            Vector3(1, 0, 0),
            Vector3(-1, 0, 0),
            Vector3(0, 1, 0),
            Vector3(0, -1, 0),
            Vector3(0, 0, 1),
            Vector3(0, 0, -1)
        };
        Vector<Vector3> file_x = {
            Vector3(0, 0, -1),
            Vector3(0, 0, 1),
            Vector3(1, 0, 0),
            Vector3(1, 0, 0),
            Vector3(1, 0, 0),
            Vector3(-1, 0, 0)
        };
        Vector<Vector3> file_y = {
            Vector3(0, -1, 0),
            Vector3(0, -1, 0),
            Vector3(0, 0, 1),
            Vector3(0, 0, -1),
            Vector3(0, -1, 0),
            Vector3(0, -1, 0)
        };

        for (int i = 0; i < 6; ++i)
        {
            const ByteBuffer& face = faces[i];
            float v = min_uv;

            for (int y = 0; y < size; ++y)
            {
                float u = min_uv;

                for (int x = 0; x < size; ++x)
                {
                    Vector3 direction = file_x[i] * u + file_y[i] * v + file_normal[i];
                    direction = Vector3::Normalize(direction);

                    float delta_solid_angle = pow(1.f + u * u + v * v, -3.f / 2.f);

                    float r = 0;
                    float g = 0;
                    float b = 0;

                    if (format == TextureFormat::R8G8B8A8)
                    {
                        int stride = 4;
                        r = face[(y * size * stride) + (x * stride) + 0];
                        g = face[(y * size * stride) + (x * stride) + 1];
                        b = face[(y * size * stride) + (x * stride) + 2];
                        r /= 255.f;
                        g /= 255.f;
                        b /= 255.f;
                    }
                    else
                    {
                        assert(!"texture format not support");
                    }

                    if (gamma_space)
                    {
                        r = pow(Mathf::Clamp01(r), Mathf::ToLinearSpace);
                        g = pow(Mathf::Clamp01(g), Mathf::ToLinearSpace);
                        b = pow(Mathf::Clamp01(b), Mathf::ToLinearSpace);
                    }

                    Color color(r, g, b, 1.0);

                    sh.AddLight(direction, color, delta_solid_angle);

                    total_solid_angle += delta_solid_angle;

                    u += du;
                }

                v += dv;
            }
        }

        float sphere_solid_angle = 4.f * Mathf::PI;
        float faces_processed = 6.f;
        float expected_solid_angle = sphere_solid_angle * faces_processed / 6.f;
        float correction_factor = expected_solid_angle / total_solid_angle;

        sh.Scale(correction_factor);
        sh.ConvertIncidentRadianceToIrradiance();
        sh.ConvertIrradianceToLambertianRadiance();

        return SphericalPolynomial::FromHarmonics(sh);
    }
}
