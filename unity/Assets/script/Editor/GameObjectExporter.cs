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

using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections.Generic;
using Newtonsoft.Json.Linq;

public class GameObjectExporter
{
    delegate void ComponentWriter(Component com);

    class Cache
    {
        public Dictionary<string, Mesh> meshes = new Dictionary<string, Mesh>();
        public Dictionary<string, Material> materials = new Dictionary<string, Material>();
        public Dictionary<string, Texture> textures = new Dictionary<string, Texture>();
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

            var prefab = PrefabUtility.GetCorrespondingObjectFromSource(obj);
            if (prefab)
            {
                string asset_path = AssetDatabase.GetAssetPath(prefab);
                if (asset_path.StartsWith("Assets/"))
                {
                    asset_path = asset_path.Substring("Assets/".Length);
                }
                string asset_dir = asset_path.Substring(0, asset_path.LastIndexOf('/'));
                go_name = asset_dir + "/" + obj.name + ".go";
            }
            else
            {
                go_name = obj.name + ".go";
            }

            WriteTransform(obj.transform);

            string file_path = out_dir + "/" + go_name;
            CreateFileDirIfNeed(file_path);

            File.WriteAllBytes(file_path, ms.ToArray());

            Debug.Log("GameObject " + obj.name + " export complete.");

            bw = null;
            cache = null;
            root = null;
        }
    }

    [MenuItem("Viry3D/Export LightMap")]
    static void WriteLightMap()
    {
        var maps = LightmapSettings.lightmaps;

        out_dir = EditorUtility.OpenFolderPanel("Select directory export to", out_dir, "");
        if (!string.IsNullOrEmpty(out_dir))
        {
            var ms = new MemoryStream();
            bw = new BinaryWriter(ms);
            cache = new Cache();

            int map_count = maps.Length;
            bw.Write(map_count);

            for (int i = 0; i < map_count; ++i)
            {
                WriteTexture(maps[i].lightmapColor);
            }

            var scene = UnityEditor.SceneManagement.EditorSceneManager.GetActiveScene();
            string scene_path = scene.path;
            if (scene_path.StartsWith("Assets/"))
            {
                scene_path = scene_path.Substring("Assets/".Length);
            }
            string scene_dir = scene_path.Substring(0, scene_path.LastIndexOf('/'));
            string scene_name = scene_dir + "/" + scene.name + ".lightmap";

            string file_path = out_dir + "/" + scene_name;
            CreateFileDirIfNeed(file_path);

            File.WriteAllBytes(file_path, ms.ToArray());

            Debug.Log("Lightmap export complete.");

            bw = null;
            cache = null;
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
            else if (coms[i] is Animation)
            {
                com_writers.Add(WriteAnimation);
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

    enum CurvePropertyType
    {
        Unknown = 0,

        LocalPositionX,
        LocalPositionY,
        LocalPositionZ,
        LocalRotationX,
        LocalRotationY,
        LocalRotationZ,
        LocalRotationW,
        LocalScaleX,
        LocalScaleY,
        LocalScaleZ,
    }

    static void WriteAnimation(Component com)
    {
        WriteString("Animation");

        var clips = AnimationUtility.GetAnimationClips(com.gameObject);

        bw.Write(clips.Length);
        for (int i = 0; i < clips.Length; ++i)
        {
            var clip = clips[i];
            var bindings = AnimationUtility.GetCurveBindings(clip);

            WriteString(clip.name);
            bw.Write(clip.length);
            bw.Write(clip.frameRate);
            bw.Write((int) clip.wrapMode);

            bw.Write(bindings.Length);
            for (int j = 0; j < bindings.Length; ++j)
            {
                var bind = bindings[j];
                var curve = AnimationUtility.GetEditorCurve(clip, bind);
                var keys = curve.keys;
                CurvePropertyType property_type = CurvePropertyType.Unknown;

                switch (bind.propertyName)
                {
                    case "m_LocalPosition.x":
                        property_type = CurvePropertyType.LocalPositionX;
                        break;
                    case "m_LocalPosition.y":
                        property_type = CurvePropertyType.LocalPositionY;
                        break;
                    case "m_LocalPosition.z":
                        property_type = CurvePropertyType.LocalPositionZ;
                        break;

                    case "m_LocalRotation.x":
                        property_type = CurvePropertyType.LocalRotationX;
                        break;
                    case "m_LocalRotation.y":
                        property_type = CurvePropertyType.LocalRotationY;
                        break;
                    case "m_LocalRotation.z":
                        property_type = CurvePropertyType.LocalRotationZ;
                        break;
                    case "m_LocalRotation.w":
                        property_type = CurvePropertyType.LocalRotationW;
                        break;

                    case "m_LocalScale.x":
                        property_type = CurvePropertyType.LocalScaleX;
                        break;
                    case "m_LocalScale.y":
                        property_type = CurvePropertyType.LocalScaleY;
                        break;
                    case "m_LocalScale.z":
                        property_type = CurvePropertyType.LocalScaleZ;
                        break;

                    default:
                        Debug.LogError(bind.propertyName);
                        break;
                }

                WriteString(bind.path);
                bw.Write((int) property_type);

                bw.Write(keys.Length);
                for (int k = 0; k < keys.Length; ++k)
                {
                    var key = keys[k];

                    bw.Write(key.time);
                    bw.Write(key.value);
                    bw.Write(key.inTangent);
                    bw.Write(key.outTangent);
                }
            }
        }
    }

    static void WriteMesh(Mesh mesh)
    {
        string asset_path = AssetDatabase.GetAssetPath(mesh) + "." + mesh.name + ".mesh";
        if (asset_path.StartsWith("Assets/"))
        {
            asset_path = asset_path.Substring("Assets/".Length);
        }
        WriteString(asset_path);

        if (cache.meshes.ContainsKey(asset_path))
        {
            return;
        }
        cache.meshes.Add(asset_path, mesh);

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

        string file_path = out_dir + "/" + asset_path;
        CreateFileDirIfNeed(file_path);

        File.WriteAllBytes(file_path, ms.ToArray());

        bw = bw_save;
    }

    static void WriteMaterial(Material material)
    {
        string asset_path = AssetDatabase.GetAssetPath(material);
        if (asset_path.StartsWith("Assets/"))
        {
            asset_path = asset_path.Substring("Assets/".Length);
        }
        else
        {
            asset_path = asset_path + "." + material.name + ".mat";
        }
        WriteString(asset_path);

        if (cache.materials.ContainsKey(asset_path))
        {
            return;
        }
        cache.materials.Add(asset_path, material);

        var bw_save = bw;
        var ms = new MemoryStream();
        bw = new BinaryWriter(ms);

        var shader = material.shader;
        int property_count = ShaderUtil.GetPropertyCount(shader);

        WriteString(material.name);
        WriteString(shader.name);
        bw.Write(property_count);

        for (int i = 0; i < property_count; ++i)
        {
            var property_name = ShaderUtil.GetPropertyName(shader, i);
            var property_type = ShaderUtil.GetPropertyType(shader, i);

            WriteString(property_name);
            bw.Write((int) property_type);

            switch (property_type)
            {
                case ShaderUtil.ShaderPropertyType.Color:
                    WriteColor32(material.GetColor(property_name));
                    break;
                case ShaderUtil.ShaderPropertyType.Vector:
                    WriteVector4(material.GetVector(property_name));
                    break;
                case ShaderUtil.ShaderPropertyType.Float:
                case ShaderUtil.ShaderPropertyType.Range:
                    bw.Write(material.GetFloat(property_name));
                    break;
                case ShaderUtil.ShaderPropertyType.TexEnv:
                    var scale = material.GetTextureScale(property_name);
                    var offset = material.GetTextureOffset(property_name);
                    
                    WriteVector4(new Vector4(scale.x, scale.y, offset.x, offset.y));

                    var texture = material.GetTexture(property_name);
                    if (texture != null)
                    {
                        WriteTexture(texture);
                    }
                    else
                    {
                        WriteString("");
                    }
                    break;
            }
        }

        string file_path = out_dir + "/" + asset_path;
        CreateFileDirIfNeed(file_path);

        File.WriteAllBytes(file_path, ms.ToArray());

        bw = bw_save;
    }

    static void WriteTexture(Texture texture)
    {
        string asset_path = AssetDatabase.GetAssetPath(texture);
        string asset_path_src = asset_path;
        if (asset_path.StartsWith("Assets/"))
        {
            asset_path = asset_path.Substring("Assets/".Length) + ".tex";
        }
        else
        {
            asset_path = asset_path + "." + texture.name + ".tex";
        }
        WriteString(asset_path);

        if (cache.textures.ContainsKey(asset_path))
        {
            return;
        }
        cache.textures.Add(asset_path, texture);

        var jtexture = new JObject();
        jtexture["name"] = texture.name;
        jtexture["width"] = texture.width;
        jtexture["height"] = texture.height;
        jtexture["wrap_mode"] = (int) texture.wrapMode;
        jtexture["filter_mode"] = (int) texture.filterMode;

        string file_path = out_dir + "/" + asset_path;
        CreateFileDirIfNeed(file_path);

        if (texture is Texture2D)
        {
            var tex2d = (texture as Texture2D);

            if (tex2d.format == TextureFormat.RGB24 ||
                tex2d.format == TextureFormat.RGBA32 ||
                tex2d.format == TextureFormat.ARGB32)
            {
                jtexture["type"] = "Texture2D";
                jtexture["mipmap"] = tex2d.mipmapCount;

                var png_path = asset_path + ".png";

                jtexture["path"] = png_path;

                png_path = out_dir + "/" + png_path;

                var importer = AssetImporter.GetAtPath(asset_path_src) as TextureImporter;
                if (importer != null && importer.textureType == TextureImporterType.NormalMap)
                {
                    // DXT5nm
                    var colors = tex2d.GetPixels();
                    var pixels = new Color32[colors.Length];
                    for (int i = 0; i < colors.Length; ++i)
                    {
                        float x = colors[i].a * 2 - 1;
                        float y = colors[i].g * 2 - 1;
                        float z = Mathf.Sqrt(1 - x * x - y * y);

                        pixels[i].r = (byte) ((x + 1) * 0.5f * 255);
                        pixels[i].g = (byte) ((y + 1) * 0.5f * 255);
                        pixels[i].b = (byte) ((z + 1) * 0.5f * 255);
                        pixels[i].a = 255;
                    }
                    var tex = new Texture2D(tex2d.width, tex2d.height, TextureFormat.RGBA32, false);
                    tex.SetPixels32(pixels);
                    var bytes = tex.EncodeToPNG();
                    File.WriteAllBytes(png_path, bytes);
                }
                else
                {
                    var bytes = tex2d.EncodeToPNG();
                    File.WriteAllBytes(png_path, bytes);
                }
            }
            else if (tex2d.format == TextureFormat.RGBAHalf)
            {
                jtexture["type"] = "Texture2DRGBFloat";
                jtexture["mipmap"] = tex2d.mipmapCount;

                var data_path = asset_path + ".f";

                jtexture["path"] = data_path;

                var colors = tex2d.GetPixels();
                var bytes = new byte[colors.Length * 12];
                for (int i = 0; i < colors.Length; ++i)
                {
                    var r = System.BitConverter.GetBytes(colors[i].r);
                    var g = System.BitConverter.GetBytes(colors[i].g);
                    var b = System.BitConverter.GetBytes(colors[i].b);

                    System.Array.Copy(r, 0, bytes, i * 12 + 0, 4);
                    System.Array.Copy(g, 0, bytes, i * 12 + 4, 4);
                    System.Array.Copy(b, 0, bytes, i * 12 + 8, 4);
                }

                data_path = out_dir + "/" + data_path;
                File.WriteAllBytes(data_path, bytes);
            }
            else
            {
                Debug.LogError("texture format not support:" + asset_path_src);
            }
        }
        else if (texture is Cubemap)
        {
            var cubemap = (texture as Cubemap);

            if (cubemap.format == TextureFormat.RGB24 ||
                cubemap.format == TextureFormat.RGBA32 ||
                cubemap.format == TextureFormat.ARGB32)
            {
                jtexture["type"] = "Cubemap";
                jtexture["mipmap"] = cubemap.mipmapCount;

                JArray jlevels = new JArray();
                jtexture["levels"] = jlevels;

                int size = cubemap.width;
                for (int i = 0; i < cubemap.mipmapCount; ++i)
                {
                    JArray jfaces = new JArray();
                    jlevels.Add(jfaces);

                    for (int j = 0; j < 6; ++j)
                    {
                        var face_path = string.Format("{0}.cubemap/{1}_{2}.png", asset_path, i, j);

                        jfaces.Add(face_path);

                        var colors = cubemap.GetPixels((CubemapFace) j, i);
                        var face = new Texture2D(size, size, cubemap.format, false);
                        for (int k = 0; k < size; ++k)
                        {
                            Color[] line = new Color[size];
                            System.Array.Copy(colors, k * size, line, 0, size);
                            face.SetPixels(0, size - k - 1, size, 1, line);
                        }

                        face_path = out_dir + "/" + face_path;
                        CreateFileDirIfNeed(face_path);

                        var bytes = face.EncodeToPNG();
                        File.WriteAllBytes(face_path, bytes);
                    }

                    size >>= 1;
                }
            }
            else if (cubemap.format == TextureFormat.RGBAHalf)
            {
                jtexture["type"] = "CubemapRGBFloat";
                jtexture["mipmap"] = cubemap.mipmapCount;

                JArray jlevels = new JArray();
                jtexture["levels"] = jlevels;

                int size = cubemap.width;
                for (int i = 0; i < cubemap.mipmapCount; ++i)
                {
                    JArray jfaces = new JArray();
                    jlevels.Add(jfaces);

                    for (int j = 0; j < 6; ++j)
                    {
                        var face_path = string.Format("{0}.cubemap/{1}_{2}.f", asset_path, i, j);

                        jfaces.Add(face_path);

                        var colors = cubemap.GetPixels((CubemapFace) j, i);
                        var bytes = new byte[colors.Length * 12];
                        for (int k = 0; k < colors.Length; ++k)
                        {
                            var r = System.BitConverter.GetBytes(colors[k].r);
                            var g = System.BitConverter.GetBytes(colors[k].g);
                            var b = System.BitConverter.GetBytes(colors[k].b);

                            System.Array.Copy(r, 0, bytes, k * 12 + 0, 4);
                            System.Array.Copy(g, 0, bytes, k * 12 + 4, 4);
                            System.Array.Copy(b, 0, bytes, k * 12 + 8, 4);
                        }

                        face_path = out_dir + "/" + face_path;
                        CreateFileDirIfNeed(face_path);

                        File.WriteAllBytes(face_path, bytes);
                    }

                    size >>= 1;
                }
            }
            else
            {
                Debug.LogError("texture format not support:" + asset_path_src);
            }
        }
        else
        {
            Debug.LogError("texture type not support:" + asset_path_src);
        }

        File.WriteAllText(file_path, jtexture.ToString());
    }
}
