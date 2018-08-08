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

#include "Resources.h"
#include "Node.h"
#include "Application.h"
#include "io/File.h"
#include "io/MemoryStream.h"
#include "graphics/MeshRenderer.h"
#include "graphics/SkinnedMeshRenderer.h"
#include "graphics/Mesh.h"

namespace Viry3D
{
    static String ReadString(MemoryStream& ms)
    {
        int size = ms.Read<int>();
        return ms.ReadString(size);
    }

    static void ReadRenderer(MemoryStream& ms, const Ref<Renderer>& renderer)
    {
        int lightmap_index = ms.Read<int>();
        Vector4 lightmapScaleOffset = ms.Read<Vector4>();
        bool cast_shadow = ms.Read<byte>() == 1;
        bool receive_shadow = ms.Read<byte>() == 1;
        
        int material_count = ms.Read<int>();
        for (int i = 0; i < material_count; ++i)
        {
            String mat_path = ReadString(ms);
        }
    }

    static void ReadMeshRenderer(MemoryStream& ms, const Ref<MeshRenderer>& renderer)
    {
        ReadRenderer(ms, renderer);

        String mesh_path = ReadString(ms);
        auto mesh = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/" + mesh_path);
        renderer->SetMesh(mesh);
    }

    static void ReadSkinnedMeshRenderer(MemoryStream& ms, const Ref<SkinnedMeshRenderer>& renderer)
    {
        ReadMeshRenderer(ms, renderer);

        int bone_count = ms.Read<int>();

        Vector<String> bones(bone_count);
        for (int i = 0; i < bone_count; ++i)
        {
            bones[i] = ReadString(ms);
        }
        renderer->SetBonePaths(bones);
    }

    static Ref<Node> ReadNode(MemoryStream& ms, const Ref<Node>& parent)
    {
        Ref<Node> node;

        String name = ReadString(ms);
        int layer = ms.Read<int>();
        bool active = ms.Read<byte>() == 1;

        Vector3 local_pos = ms.Read<Vector3>();
        Quaternion local_rot = ms.Read<Quaternion>();
        Vector3 local_scale = ms.Read<Vector3>();

        int com_count = ms.Read<int>();
        for (int i = 0; i < com_count; ++i)
        {
            String com_name = ReadString(ms);

            if (com_name == "MeshRenderer")
            {
                assert(!node);

                auto renderer = RefMake<MeshRenderer>();
                ReadMeshRenderer(ms, renderer);
                node = renderer;
            }
            else if (com_name == "SkinnedMeshRenderer")
            {
                assert(!node);

                auto renderer = RefMake<SkinnedMeshRenderer>();
                ReadSkinnedMeshRenderer(ms, renderer);
                node = renderer;

                if (parent)
                {
                    renderer->SetBonesRoot(Node::GetRoot(parent));
                }
                else
                {
                    renderer->SetBonesRoot(renderer);
                }
            }
        }

        if (!node)
        {
            node = RefMake<Node>();
        }

        if (parent)
        {
            Node::SetParent(node, parent);
        }

        node->SetName(name);
        node->SetLocalPosition(local_pos);
        node->SetLocalRotation(local_rot);
        node->SetLocalScale(local_scale);

        int child_count = ms.Read<int>();
        for (int i = 0; i < child_count; ++i)
        {
            ReadNode(ms, node);
        }

        return node;
    }

    Ref<Node> Resources::Load(const String& path)
    {
        Ref<Node> node;

        if (File::Exist(path))
        {
            MemoryStream ms(File::ReadAllBytes(path));

            node = ReadNode(ms, Ref<Node>());
        }

        return node;
    }
}
