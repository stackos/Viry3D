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
using Newtonsoft.Json.Linq;

public class AnimationTool
{
    static string out_dir;

    [MenuItem("Viry3D/Export Bones")]
    static void ExportBones()
    {
        var obj = Selection.activeGameObject;
        if (obj == null)
        {
            return;
        }

        out_dir = EditorUtility.OpenFolderPanel("Select directory export to", out_dir, "");
        if (!string.IsNullOrEmpty(out_dir))
        {
            var jbone = new JObject();
            WriteTransform(obj.transform, jbone);

            string file_path = out_dir + "/bones.json";
            CreateFileDirIfNeed(file_path);

            File.WriteAllText(file_path, jbone.ToString());

            Debug.Log("Bones export complete.");
        }
    }

    static void WriteTransform(Transform t, JObject jbone)
    {
        var obj = t.gameObject;

        var jpos = new JObject();
        jpos["x"] = t.localPosition.x;
        jpos["y"] = t.localPosition.y;
        jpos["z"] = t.localPosition.z;

        var jrot = new JObject();
        jrot["x"] = t.localRotation.x;
        jrot["y"] = t.localRotation.y;
        jrot["z"] = t.localRotation.z;
        jrot["w"] = t.localRotation.w;

        var jsca = new JObject();
        jsca["x"] = t.localScale.x;
        jsca["y"] = t.localScale.y;
        jsca["z"] = t.localScale.z;

        int child_count = t.childCount;
        var jchildren = new JArray();

        jbone["name"] = obj.name;
        jbone["localPosition"] = jpos;
        jbone["localRotation"] = jrot;
        jbone["localScale"] = jsca;
        jbone["childCount"] = child_count;
        jbone["children"] = jchildren;

        for (int i = 0; i < child_count; ++i)
        {
            var jchild = new JObject();
            jchildren.Add(jchild);

            WriteTransform(t.GetChild(i), jchild);
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
}
