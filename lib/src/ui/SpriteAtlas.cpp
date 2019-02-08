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

#include "SpriteAtlas.h"
#include "io/File.h"
#include "json/json.h"
#include "graphics/Texture.h"

namespace Viry3D
{
    Ref<SpriteAtlas> SpriteAtlas::LoadFromFile(const String& file)
    {
        Ref<SpriteAtlas> atlas;

        if (File::Exist(file))
        {
            String json = File::ReadAllText(file);

            auto reader = Ref<Json::CharReader>(Json::CharReaderBuilder().newCharReader());
            Json::Value root;
            const char* begin = json.CString();
            const char* end = begin + json.Size();
            if (reader->parse(begin, end, &root, nullptr))
            {
                atlas = RefMake<SpriteAtlas>();

                String texture_path = root["texture"].asCString();
                texture_path = file.Substring(0, file.LastIndexOf("/") + 1) + texture_path;
                atlas->m_texture = Texture::LoadTexture2DFromFile(texture_path, FilterMode::Nearest, SamplerAddressMode::ClampToEdge, false, false);
            
                const Json::Value& sprites = root["sprites"];
                for (int i = 0; i < (int) sprites.size(); ++i)
                {
                    Sprite sprite;

                    const Json::Value& s = sprites[i];
                    sprite.name = s["name"].asCString();
                    const Json::Value& rect = s["rect"];
                    const Json::Value& border = s["border"];
                    sprite.rect = Recti((int) rect[0].asFloat(), (int) rect[1].asFloat(), (int) rect[2].asFloat(), (int) rect[3].asFloat());
                    sprite.border = Vector4(border[0].asFloat(), border[1].asFloat(), border[2].asFloat(), border[3].asFloat());
                
                    atlas->m_sprites.Add(sprite.name, sprite);
                }
            }
        }

        return atlas;
    }

    SpriteAtlas::SpriteAtlas()
    {
    
    }

    SpriteAtlas::~SpriteAtlas()
    {
        
    }
}
