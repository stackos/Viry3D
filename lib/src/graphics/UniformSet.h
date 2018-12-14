/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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

#include "container/Vector.h"
#include "string/String.h"
#include "memory/Ref.h"

namespace Viry3D
{
    class BufferObject;

    struct UniformMember
    {
        String name;
        int offset;
        int size;
    };

    struct UniformBuffer
    {
        String name;
        int binding;
        int stage;
        Vector<UniformMember> members;
        int size;
        Ref<BufferObject> buffer;
    };

    struct UniformTexture
    {
        String name;
        int binding;
        int stage;
    };

    struct StorageBuffer
    {
        String name;
        int binding;
        int stage;
    };

    struct UniformTexelBuffer
    {
        String name;
        int binding;
        int stage;
    };

    struct StorageTexelBuffer
    {
        String name;
        int binding;
        int stage;
    };

    struct UniformSet
    {
        int set;
        Vector<UniformBuffer> buffers;
        Vector<UniformTexture> textures;
        Vector<StorageBuffer> storage_buffers;
        Vector<UniformTexelBuffer> uniform_texel_buffers;
        Vector<StorageTexelBuffer> storage_texel_buffers;
    };
}
