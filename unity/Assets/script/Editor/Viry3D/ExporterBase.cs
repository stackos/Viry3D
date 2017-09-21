using UnityEditor;
using UnityEngine;
using System.IO;
using System.Text;
using System.Collections.Generic;

public class ExporterBase {
    protected static BinaryWriter m_writer;
    protected static Dictionary<string, Object> m_cache;
    protected static string m_out_path;

    protected static MemoryStream Init() {
        m_cache = new Dictionary<string, Object>();
        m_out_path = Application.dataPath + "/export out";

        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        return ms;
    }

    protected static void Deinit() {
        m_writer.Close();
        m_writer = null;

        m_cache.Clear();
        m_cache = null;

        m_out_path = null;
    }

	protected static void WriteVector2(Vector2 v) {
        m_writer.Write(v.x);
        m_writer.Write(v.y);
    }

    protected static void WriteVector3(Vector3 v) {
        m_writer.Write(v.x);
        m_writer.Write(v.y);
        m_writer.Write(v.z);
    }

    protected static void WriteVector4(Vector4 v) {
        m_writer.Write(v.x);
        m_writer.Write(v.y);
        m_writer.Write(v.z);
        m_writer.Write(v.w);
    }

    protected static void WriteRect(Rect v) {
        m_writer.Write(v.x);
        m_writer.Write(v.y);
        m_writer.Write(v.width);
        m_writer.Write(v.height);
    }

    protected static void WriteQuaternion(Quaternion v) {
        m_writer.Write(v.x);
        m_writer.Write(v.y);
        m_writer.Write(v.z);
        m_writer.Write(v.w);
    }

    protected static void WriteColor(Color v) {
        m_writer.Write(v.r);
        m_writer.Write(v.g);
        m_writer.Write(v.b);
        m_writer.Write(v.a);
    }

    protected static void WriteString(string v) {
        var bytes = Encoding.UTF8.GetBytes(v);
        m_writer.Write(bytes.Length);
        if(bytes.Length > 0) {
            m_writer.Write(bytes);
        }
    }

    protected static void Log(object obj) {
        Debug.Log(obj);
    }

    protected static void WriteMaterial(Material mat) {
        var path = AssetDatabase.GetAssetPath(mat);

        WriteString(path);

        if(m_cache.ContainsKey(path)) {
            return;
        } else {
            m_cache.Add(path, mat);
        }

        var save = m_writer;
        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        var shader = mat.shader;
        int property_count = ShaderUtil.GetPropertyCount(shader);

        WriteString(mat.name);
        WriteString(shader.name);
        m_writer.Write(property_count);

        for(int i = 0; i < property_count; i++) {
            var property_name = ShaderUtil.GetPropertyName(shader, i);
            var property_type = ShaderUtil.GetPropertyType(shader, i);

            WriteString(property_name);
            WriteString(property_type.ToString());

            switch(property_type) {
            case ShaderUtil.ShaderPropertyType.Color:
                WriteColor(mat.GetColor(property_name));
                break;
            case ShaderUtil.ShaderPropertyType.Vector:
                WriteVector4(mat.GetVector(property_name));
                break;
            case ShaderUtil.ShaderPropertyType.Float:
            case ShaderUtil.ShaderPropertyType.Range:
                m_writer.Write(mat.GetFloat(property_name));
                break;
            case ShaderUtil.ShaderPropertyType.TexEnv:
				var scale = mat.GetTextureScale(property_name);
				var offset = mat.GetTextureOffset(property_name);
				WriteVector4(new Vector4(scale.x, scale.y, offset.x, offset.y));

                var texture = mat.GetTexture(property_name);
                if(texture != null) {
                    WriteTexture(texture);
                } else {
                    WriteString("");
                }
                break;
            }
        }

        var file_path = new FileInfo(m_out_path + "/" + path);
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }
        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        m_writer.Close();
        m_writer = save;
    }

    protected static void WriteTexture(Texture texture) {
        var path = AssetDatabase.GetAssetPath(texture) + ".tex";

        WriteString(path);

        if(m_cache.ContainsKey(path)) {
            return;
        } else {
            m_cache.Add(path, texture);
        }

        var file_path = new FileInfo(m_out_path + "/" + path);
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }

        var save = m_writer;
        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        WriteString(texture.name);
        m_writer.Write(texture.width);
        m_writer.Write(texture.height);
        m_writer.Write((int) texture.wrapMode);
        m_writer.Write((int) texture.filterMode);

        if(texture is Texture2D) {
            WriteString("Texture2D");
            m_writer.Write((texture as Texture2D).mipmapCount);

            var png_path = path + ".png";

            WriteString(png_path);

            png_path = m_out_path + "/" + png_path;

            var tex2d = texture as Texture2D;
			var bytes = tex2d.EncodeToPNG();
			File.WriteAllBytes(png_path, bytes);
		} else {
            WriteString("Texture");
        }

        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        m_writer.Close();
        m_writer = save;
    }
}