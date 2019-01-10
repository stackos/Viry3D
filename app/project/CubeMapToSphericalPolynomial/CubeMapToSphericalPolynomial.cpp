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

#include "json/json.h"
#include "graphics/Texture.h"
#include "graphics/CubeMapToSphericalPolynomialTools.h"
#include "io/File.h"

using namespace Viry3D;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage:\n");
        printf("\tCubeMapToSphericalPolynomial.exe input.json\n");
        return 0;
    }

    std::string input = argv[1];
    std::string input_buffer = File::ReadAllText(input.c_str()).CString();
    
    auto reader = Ref<Json::CharReader>(Json::CharReaderBuilder().newCharReader());
    Json::Value root;
    const char* begin = input_buffer.c_str();
    const char* end = begin + input_buffer.size();
    if (reader->parse(begin, end, &root, nullptr))
    {
        auto cubemap_faces = root["cubemap_faces"];
        auto format = root["format"];
        auto gamma_space = root["gamma_space"];
        auto output = root["output"];

        if (cubemap_faces.isArray() && cubemap_faces.size() == 6 && format.isString() && gamma_space.isBool() && output.isString())
        {
            TextureFormat texture_format = TextureFormat::None;
            if (format.asString() == "R8G8B8A8")
            {
                texture_format = TextureFormat::R8G8B8A8;
            }

            if (texture_format != TextureFormat::None)
            {
                int width = 0;
                int height = 0;
                int bpp = 0;
                Vector<ByteBuffer> faces(6);
                for (int i = 0; i < faces.Size(); ++i)
                {
                    ByteBuffer pixels = Texture::LoadImageFromFile(cubemap_faces[i].asCString(), width, height, bpp);
                    if (pixels.Size() == 0)
                    {
                        return 0;
                    }
                    faces[i] = pixels;
                }

                SphericalPolynomial sp = CubeMapToSphericalPolynomialTools::ConvertCubeMapToSphericalPolynomial(
                    width,
                    texture_format,
                    faces,
                    gamma_space.asBool());

                Json::Value x;
                Json::Value y;
                Json::Value z;
                Json::Value xx;
                Json::Value yy;
                Json::Value zz;
                Json::Value xy;
                Json::Value yz;
                Json::Value zx;

                x[0] = sp.x.x;
                x[1] = sp.x.y;
                x[2] = sp.x.z;

                y[0] = sp.y.x;
                y[1] = sp.y.y;
                y[2] = sp.y.z;

                z[0] = sp.z.x;
                z[1] = sp.z.y;
                z[2] = sp.z.z;

                xx[0] = sp.xx.x;
                xx[1] = sp.xx.y;
                xx[2] = sp.xx.z;

                yy[0] = sp.yy.x;
                yy[1] = sp.yy.y;
                yy[2] = sp.yy.z;

                zz[0] = sp.zz.x;
                zz[1] = sp.zz.y;
                zz[2] = sp.zz.z;

                xy[0] = sp.xy.x;
                xy[1] = sp.xy.y;
                xy[2] = sp.xy.z;

                yz[0] = sp.yz.x;
                yz[1] = sp.yz.y;
                yz[2] = sp.yz.z;

                zx[0] = sp.zx.x;
                zx[1] = sp.zx.y;
                zx[2] = sp.zx.z;

                Json::Value out_sp;
                out_sp["x"] = x;
                out_sp["y"] = y;
                out_sp["z"] = z;
                out_sp["xx"] = xx;
                out_sp["yy"] = yy;
                out_sp["zz"] = zz;
                out_sp["xy"] = xy;
                out_sp["yz"] = yz;
                out_sp["zx"] = zx;

                std::string out_json = out_sp.toStyledString();
                File::WriteAllText(output.asCString(), out_json.c_str());
            }
        }
    }

    return 0;
}
