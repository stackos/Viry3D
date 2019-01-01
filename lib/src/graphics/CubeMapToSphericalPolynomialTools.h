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

#pragma once

#include "math/Vector3.h"
#include "graphics/Color.h"
#include "graphics/Texture.h"

namespace Viry3D
{
    class SphericalHarmonics
    {
    public:
        SphericalHarmonics();
        void AddLight(const Vector3& direction, const Color& color, float delta_solid_angle);
        void Scale(float scale);
        void ConvertIncidentRadianceToIrradiance();
        void ConvertIrradianceToLambertianRadiance();

        Vector3 l00;
        Vector3 l1_1;
        Vector3 l10;
        Vector3 l11;
        Vector3 l2_2;
        Vector3 l2_1;
        Vector3 l20;
        Vector3 l21;
        Vector3 lL22;
    };

    class SphericalPolynomial
    {
    public:
        static SphericalPolynomial FromHarmonics(const SphericalHarmonics& sh);
        SphericalPolynomial();
        void Scale(float scale);

        Vector3 x;
        Vector3 y;
        Vector3 z;
        Vector3 xx;
        Vector3 yy;
        Vector3 zz;
        Vector3 xy;
        Vector3 yz;
        Vector3 zx;
    };

    class CubeMapToSphericalPolynomialTools
    {
    public:
        static SphericalPolynomial ConvertCubeMapToSphericalPolynomial(int size, TextureFormat format, const Vector<ByteBuffer>& faces, bool gamma_space);
    };
}
