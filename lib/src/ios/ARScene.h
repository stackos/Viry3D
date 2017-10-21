/*
 * Viry3D
 * Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "memory/Ref.h"

namespace Viry3D
{
    class Texture2D;
    
    class ARScene
    {
    public:
        static bool IsSupported();
        ARScene();
        virtual ~ARScene();
        void RunSession();
        void PauseSession();
        void UpdateSession();
        const Ref<Texture2D>& GetBackgroundTextureY() { return m_background_texture_y; }
        const Ref<Texture2D>& GetBackgroundTextureUV() { return m_background_texture_uv; }
        
    private:
        Ref<Texture2D> m_background_texture_y;
        Ref<Texture2D> m_background_texture_uv;
    };
}
