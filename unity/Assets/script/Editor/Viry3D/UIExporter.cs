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

using UnityEditor;
using UnityEngine;
using UnityEngine.UI;
using System.IO;

public class UIExporter : ExporterBase {
    [MenuItem("Viry3D/Export/UI")]
    public static void ExportUI() {
        var obj = Selection.activeGameObject;
        if(obj == null) {
            return;
        }

        MemoryStream ms = Init();

		WriteVersion();

		WriteTransform(obj.transform);

        var file_path = new FileInfo(m_out_path + "/Assets/" + obj.name + ".prefab");
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }
        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        Deinit();

        Log("export ui done.");
    }

    static void WriteRectTransform(RectTransform rt) {
        WriteVector2(rt.anchorMin);
        WriteVector2(rt.anchorMax);
        WriteVector2(rt.offsetMin);
        WriteVector2(rt.offsetMax);
        WriteVector2(rt.pivot);
    }

    static void WriteTransform(Transform t) {
        WriteString(t.name);
		m_writer.Write(t.gameObject.layer);
		m_writer.Write(t.gameObject.activeSelf);
		m_writer.Write(false);
		
		WriteVector3(t.localPosition);
        WriteQuaternion(t.localRotation);
        WriteVector3(t.localScale);
        m_writer.Write(t.GetInstanceID());

        int com_count = 0;
        if(t is RectTransform) {
            var canvas = t.GetComponent<Canvas>();
            var ui = t.GetComponent<MaskableGraphic>();
            string type_name = "";

            if(canvas != null) {
                type_name = "Canvas";
            } else if(ui != null) {
                if(ui is Image) {
                    type_name = "Image";
                } else if(ui is Text) {
                    type_name = "Text";
                }
            } else {
				type_name = "RectTransform";
			}

            com_count = 1;
            
            m_writer.Write(com_count);

            WriteString(type_name);

            WriteRectTransform(t as RectTransform);

            if(!string.IsNullOrEmpty(type_name)) {
                switch(type_name) {
                case "Canvas":
                    m_writer.Write(canvas.sortingOrder);
                    break;
                case "Image":
                    WriteImage(ui as Image);
                    break;
                case "Text":
                    WriteText(ui as Text);
                    break;
                }
            }
        } else {
            m_writer.Write(com_count);
        }

        int child_count = t.childCount;
        m_writer.Write(child_count);

        for(int i = 0; i < child_count; i++) {
            var child = t.GetChild(i);

            WriteTransform(child);
        }
    }

    static void WriteImage(Image image) {
        WriteColor(image.color);
        m_writer.Write((int) image.type);
        m_writer.Write((int) image.fillMethod);
        m_writer.Write(image.fillOrigin);
        m_writer.Write(image.fillAmount);
        m_writer.Write(image.fillClockwise);

        if(image.sprite != null) {
            WriteSprite(image.sprite);
        } else {
            WriteString("");
        }
    }

    static void WriteSprite(Sprite sprite) {
        WriteString(sprite.name);

        var path = AssetDatabase.GetAssetPath(sprite) + ".atlas";
        WriteString(path);

        if(m_cache.ContainsKey(path)) {
            return;
        } else {
            m_cache.Add(path, null);
        }

        var save = m_writer;
        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        var sprite_path = AssetDatabase.GetAssetPath(sprite);
        var assets = AssetDatabase.LoadAllAssetsAtPath(sprite_path);
        var tex = AssetDatabase.LoadAssetAtPath<Texture2D>(sprite_path);

        WriteTexture(tex);

        int sprite_count = assets.Length - 1;

        m_writer.Write(sprite_count);

        for(int i = 0; i < assets.Length; i++) {
            if(!(assets[i] is Sprite)) {
                continue;
            }

            var s = assets[i] as Sprite;

            WriteString(s.name);
            var rect = s.rect;
            rect.x /= tex.width;
            rect.y /= tex.height;
            rect.width /= tex.width;
            rect.height /= tex.height;
            WriteRect(rect);
            WriteVector2(s.pivot);
            m_writer.Write(s.pixelsPerUnit);
            WriteVector4(s.border);
        }

        var file_path = new FileInfo(m_out_path + "/" + path);
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }
        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        m_writer.Close();
        m_writer = save;
    }

    static void WriteText(Text text) {
        WriteColor(text.color);
        WriteString(text.text);

        if(text.font != null) {
            WriteString(text.font.name);
        } else {
            WriteString("");
        }

        WriteString(text.fontStyle.ToString());
        m_writer.Write(text.fontSize);
        m_writer.Write(text.lineSpacing);
        m_writer.Write(text.supportRichText);
        WriteString(text.alignment.ToString());
    }
}