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

using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections.Generic;

public class GameObjectExporter
{
    delegate void ComponentWriter(Component com);

    class Cache
    {
        public Dictionary<string, Mesh> meshes = new Dictionary<string, Mesh>();
        public Dictionary<string, Material> materials = new Dictionary<string, Material>();
    }

    static BinaryWriter bw;
    static string out_dir;
    static Cache cache;
    static Transform root;

    [MenuItem("Viry3D/Export GameObject")]
    public static void Export()
    {
        var obj = Selection.activeGameObject;
        if (obj == null)
        {
            return;
        }

        out_dir = EditorUtility.OpenFolderPanel("Select directory export to", out_dir, "");
        if (!string.IsNullOrEmpty(out_dir))
        {
            var ms = new MemoryStream();
            bw = new BinaryWriter(ms);
            cache = new Cache();
            root = obj.transform;

            string go_name;

            var prefab = PrefabUtility.GetPrefabParent(obj);
            if (prefab)
            {
                string prefab_path = AssetDatabase.GetAssetPath(prefab);
                if (prefab_path.StartsWith("Assets/"))
                {
                    prefab_path = prefab_path.Substring("Assets/".Length);
                }
                string prefab_dir = prefab_path.Substring(0, prefab_path.LastIndexOf('/'));
                go_name = prefab_dir + "/" + obj.name + ".go";
            }
            else
            {
                go_name = obj.name + ".go";
            }

            WriteTransform(obj.transform);

            string go_path = out_dir + "/" + go_name;
            CreateFileDirIfNeed(go_path);

            File.WriteAllBytes(go_path, ms.ToArray());

            Debug.Log("GameObject " + obj.name + " export complete.");

            bw = null;
            cache = null;
            root = null;
        }
    }

    static void CreateFileDirIfNeed(string path)
    {
        var dir = new FileInfo(path).Directory;
        if (!dir.Exists)
        {
            dir.Create();
        }
    }

    static void WriteTransform(Transform t)
    {
        var obj = t.gameObject;

        WriteString(obj.name);
        bw.Write(obj.layer);
        bw.Write((byte) (obj.activeSelf ? 1 : 0));

        WriteVector3(t.localPosition);
        WriteQuaternion(t.localRotation);
        WriteVector3(t.localScale);

        WriteComponents(obj);

        int child_count = t.childCount;
        bw.Write(child_count);

        for (int i = 0; i < child_count; ++i)
        {
            WriteTransform(t.GetChild(i));
        }
    }

    static void WriteComponents(GameObject obj)
    {
        List<ComponentWriter> com_writers = new List<ComponentWriter>();
        List<Component> write_coms = new List<Component>();

        var coms = obj.GetComponents<Component>();
        for (int i = 0; i < coms.Length; ++i)
        {
            if (coms[i] is MeshRenderer)
            {
                com_writers.Add(WriteMeshRenderer);
                write_coms.Add(coms[i]);
            }
            else if (coms[i] is SkinnedMeshRenderer)
            {
                com_writers.Add(WriteSkinnedMeshRenderer);
                write_coms.Add(coms[i]);
            }
        }

        bw.Write(write_coms.Count);

        for (int i = 0; i < write_coms.Count; ++i)
        {
            com_writers[i].Invoke(write_coms[i]);
        }
    }

    static void WriteString(string v)
    {
        var bytes = Encoding.UTF8.GetBytes(v);
        bw.Write(bytes.Length);
        bw.Write(bytes);
    }

    static void WriteVector2(Vector2 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
    }

    static void WriteVector3(Vector3 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
    }

    static void WriteVector4(Vector4 v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
        bw.Write(v.w);
    }

    static void WriteQuaternion(Quaternion v)
    {
        bw.Write(v.x);
        bw.Write(v.y);
        bw.Write(v.z);
        bw.Write(v.w);
    }

    static void WriteColor32(Color32 v)
    {
        bw.Write(v.r);
        bw.Write(v.g);
        bw.Write(v.b);
        bw.Write(v.a);
    }

    static void WriteRenderer(Component com)
    {
        var renderer = com as Renderer;
        var materials = renderer.sharedMaterials;

        bw.Write(renderer.lightmapIndex);
        WriteVector4(renderer.lightmapScaleOffset);
        bw.Write((byte) (renderer.shadowCastingMode == UnityEngine.Rendering.ShadowCastingMode.Off ? 0 : 1));
        bw.Write((byte) (renderer.receiveShadows ? 1 : 0));

        bw.Write(materials.Length);
        for (int i = 0; i < materials.Length; ++i)
        {
            if (materials[i])
            {
                WriteMaterial(materials[i]);
            }
            else
            {
                WriteString("");
            }
        }
    }

    static void WriteMeshRenderer(Component com)
    {
        WriteString("MeshRenderer");

        WriteRenderer(com);

        var filter = com.GetComponent<MeshFilter>();
        if (filter == null)
        {
            WriteString("");
        }
        else
        {
            var mesh = filter.sharedMesh;
            if (mesh == null)
            {
                WriteString("");
            }
            else
            {
                WriteMesh(mesh);
            }
        }
    }

    static string TransformPath(Transform t, Transform root)
    {
        string path = t.name;
        Transform p = t.parent;
        while (p != null)
        {
            path = p.name + '/' + path;
            if (p == root)
            {
                return path;
            }

            p = p.parent;
        }

        return "";
    }

    static void WriteSkinnedMeshRenderer(Component com)
    {
        WriteString("SkinnedMeshRenderer");

        WriteRenderer(com);

        var skin = com as SkinnedMeshRenderer;
        var mesh = skin.sharedMesh;
        var bones = skin.bones;

        if (mesh == null)
        {
            WriteString("");
        }
        else
        {
            WriteMesh(mesh);
        }

        bw.Write(bones.Length);
        for (int i = 0; i < bones.Length; ++i)
        {
            string bone_path = TransformPath(bones[i], root);
            WriteString(bone_path);
        }
    }

    static void WriteMesh(Mesh mesh)
    {
        string mesh_path = AssetDatabase.GetAssetPath(mesh) + "." + mesh.name + ".mesh";
        if (mesh_path.StartsWith("Assets/"))
        {
            mesh_path = mesh_path.Substring("Assets/".Length);
        }
        WriteString(mesh_path);

        if (cache.meshes.ContainsKey(mesh_path))
        {
            return;
        }
        cache.meshes.Add(mesh_path, mesh);

        var bw_save = bw;
        var ms = new MemoryStream();
        bw = new BinaryWriter(ms);

        WriteString(mesh.name);

        var vertices = mesh.vertices;
        var colors = mesh.colors32;
        var uv = mesh.uv;
        var uv2 = mesh.uv2;
        var normals = mesh.normals;
        var tangents = mesh.tangents;
        var boneWeights = mesh.boneWeights;
        var triangles = mesh.triangles;
        var bindposes = mesh.bindposes;

        bw.Write(vertices.Length);
        for (int i = 0; i < vertices.Length; ++i)
        {
            WriteVector3(vertices[i]);
        }

        bw.Write(colors.Length);
        for (int i = 0; i < colors.Length; ++i)
        {
            WriteColor32(colors[i]);
        }

        bw.Write(uv.Length);
        for (int i = 0; i < uv.Length; ++i)
        {
            Vector2 v = new Vector2(uv[i].x, 1.0f - uv[i].y);
            WriteVector2(v);
        }

        bw.Write(uv2.Length);
        for (int i = 0; i < uv2.Length; ++i)
        {
            Vector2 v = new Vector2(uv2[i].x, 1.0f - uv2[i].y);
            WriteVector2(v);
        }

        bw.Write(normals.Length);
        for (int i = 0; i < normals.Length; ++i)
        {
            WriteVector3(normals[i]);
        }

        bw.Write(tangents.Length);
        for (int i = 0; i < tangents.Length; ++i)
        {
            WriteVector4(tangents[i]);
        }

        bw.Write(boneWeights.Length);
        for (int i = 0; i < boneWeights.Length; ++i)
        {
            bw.Write(boneWeights[i].weight0);
            bw.Write(boneWeights[i].weight1);
            bw.Write(boneWeights[i].weight2);
            bw.Write(boneWeights[i].weight3);
            bw.Write((byte) boneWeights[i].boneIndex0);
            bw.Write((byte) boneWeights[i].boneIndex1);
            bw.Write((byte) boneWeights[i].boneIndex2);
            bw.Write((byte) boneWeights[i].boneIndex3);
        }

        bw.Write(triangles.Length);
        int face_count = triangles.Length / 3;
        for (int i = 0; i < face_count; ++i)
        {
            bw.Write((ushort) triangles[i * 3 + 0]);
            bw.Write((ushort) triangles[i * 3 + 2]);
            bw.Write((ushort) triangles[i * 3 + 1]);
        }

        bw.Write(mesh.subMeshCount);
        for (int i = 0; i < mesh.subMeshCount; ++i)
        {
            bw.Write(mesh.GetIndexStart(i));
            bw.Write(mesh.GetIndexCount(i));
        }

        bw.Write(bindposes.Length);
        for (int i = 0; i < bindposes.Length; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                for (int k = 0; k < 4; ++k)
                {
                    bw.Write(bindposes[i][j, k]);
                }
            }
        }

        string mesh_file_path = out_dir + "/" + mesh_path;
        CreateFileDirIfNeed(mesh_file_path);

        File.WriteAllBytes(mesh_file_path, ms.ToArray());

        bw = bw_save;
    }

    static void WriteMaterial(Material material)
    {
        string mat_path = AssetDatabase.GetAssetPath(material);
        if (mat_path.StartsWith("Assets/"))
        {
            mat_path = mat_path.Substring("Assets/".Length);
        }
        else
        {
            mat_path = mat_path + "." + material.name + ".mat";
        }
        WriteString(mat_path);

        if (cache.materials.ContainsKey(mat_path))
        {
            return;
        }
        cache.materials.Add(mat_path, material);

        var bw_save = bw;
        var ms = new MemoryStream();
        bw = new BinaryWriter(ms);

        WriteString(material.name);
        WriteString(material.shader.name);

        string mat_file_path = out_dir + "/" + mat_path;
        CreateFileDirIfNeed(mat_file_path);

        File.WriteAllBytes(mat_file_path, ms.ToArray());

        bw = bw_save;
    }
}
