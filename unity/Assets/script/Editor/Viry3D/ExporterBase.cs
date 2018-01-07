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
using System.IO;
using System.Text;
using System.Collections.Generic;

public class ExporterBase
{
	protected static BinaryWriter m_writer;
	protected static Dictionary<string, Object> m_cache;
	protected static string m_out_path;

	protected static MemoryStream Init()
	{
		m_cache = new Dictionary<string, Object>();
		m_out_path = Application.dataPath + "/export out";

		var ms = new MemoryStream();
		m_writer = new BinaryWriter(ms);

		return ms;
	}

	protected static void Deinit()
	{
		m_writer.Close();
		m_writer = null;

		m_cache.Clear();
		m_cache = null;

		m_out_path = null;
	}

	protected static void WriteVersion()
	{
		m_writer.Write((byte) 'V');
		m_writer.Write((byte) 'I');
		m_writer.Write((byte) 'R');
		m_writer.Write((byte) 'Y');
		m_writer.Write(0x00010000); // version 1.0
	}

	protected static void WriteVector2(Vector2 v)
	{
		m_writer.Write(v.x);
		m_writer.Write(v.y);
	}

	protected static void WriteVector3(Vector3 v)
	{
		m_writer.Write(v.x);
		m_writer.Write(v.y);
		m_writer.Write(v.z);
	}

	protected static void WriteVector4(Vector4 v)
	{
		m_writer.Write(v.x);
		m_writer.Write(v.y);
		m_writer.Write(v.z);
		m_writer.Write(v.w);
	}

	protected static void WriteRect(Rect v)
	{
		m_writer.Write(v.x);
		m_writer.Write(v.y);
		m_writer.Write(v.width);
		m_writer.Write(v.height);
	}

	protected static void WriteQuaternion(Quaternion v)
	{
		m_writer.Write(v.x);
		m_writer.Write(v.y);
		m_writer.Write(v.z);
		m_writer.Write(v.w);
	}

	protected static void WriteColor(Color v)
	{
		m_writer.Write(v.r);
		m_writer.Write(v.g);
		m_writer.Write(v.b);
		m_writer.Write(v.a);
	}

	protected static void WriteString(string v)
	{
		var bytes = Encoding.UTF8.GetBytes(v);
		m_writer.Write(bytes.Length);
		if (bytes.Length > 0)
		{
			m_writer.Write(bytes);
		}
	}

	protected static void Log(object obj)
	{
		Debug.Log(obj);
	}

	protected static void WriteMaterial(Material mat)
	{
		var path = AssetDatabase.GetAssetPath(mat);

		WriteString(path);

		if (m_cache.ContainsKey(path))
		{
			return;
		}
		else
		{
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

		for (int i = 0; i < property_count; i++)
		{
			var property_name = ShaderUtil.GetPropertyName(shader, i);
			var property_type = ShaderUtil.GetPropertyType(shader, i);

			WriteString(property_name);
			WriteString(property_type.ToString());

			switch (property_type)
			{
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

		var file_path = new FileInfo(m_out_path + "/" + path);
		if (!file_path.Directory.Exists)
		{
			Directory.CreateDirectory(file_path.Directory.FullName);
		}
		File.WriteAllBytes(file_path.FullName, ms.ToArray());

		m_writer.Close();
		m_writer = save;
	}

	protected static void WriteTexture(Texture texture, int index = -1)
	{
		var asset_path = AssetDatabase.GetAssetPath(texture);
		string path;
		if (index >= 0)
		{
			path = string.Format("{0}.{1}.tex", asset_path, index);
		}
		else
		{
			path = asset_path + ".tex";
		}

		WriteString(path);

		if (m_cache.ContainsKey(path))
		{
			return;
		}
		else
		{
			m_cache.Add(path, texture);
		}

		var file_path = new FileInfo(m_out_path + "/" + path);
		if (!file_path.Directory.Exists)
		{
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

		if (texture is Texture2D)
		{
			var tex2d = (texture as Texture2D);

			if (tex2d.format == TextureFormat.RGB24 ||
				tex2d.format == TextureFormat.RGBA32 ||
				tex2d.format == TextureFormat.ARGB32)
			{
				WriteString("Texture2D");
				m_writer.Write(tex2d.mipmapCount);

				var png_path = path + ".png";

				WriteString(png_path);

				png_path = m_out_path + "/" + png_path;

				var importer = AssetImporter.GetAtPath(asset_path) as TextureImporter;
				if (importer != null && importer.textureType == TextureImporterType.NormalMap)
				{
					if (asset_path.EndsWith(".png") || asset_path.EndsWith(".PNG"))
					{
						File.Copy(asset_path, png_path, true);
					}
					else
					{
						Debug.LogError("need png format normal map:" + asset_path);
					}
				}
				else
				{
					var bytes = tex2d.EncodeToPNG();
					File.WriteAllBytes(png_path, bytes);
				}
			}
			else if (tex2d.format == TextureFormat.RGBAHalf)
			{
				WriteString("Texture2DRGBFloat");
				m_writer.Write(tex2d.mipmapCount);

				var data_path = path + ".f";

				WriteString(data_path);

				var colors = tex2d.GetPixels();

				data_path = m_out_path + "/" + data_path;

				var bytes = new byte[colors.Length * 12];
				for (int k = 0; k < colors.Length; k++)
				{
					var r = System.BitConverter.GetBytes(colors[k].r);
					var g = System.BitConverter.GetBytes(colors[k].g);
					var b = System.BitConverter.GetBytes(colors[k].b);

					System.Array.Copy(r, 0, bytes, k * 12 + 0, 4);
					System.Array.Copy(g, 0, bytes, k * 12 + 4, 4);
					System.Array.Copy(b, 0, bytes, k * 12 + 8, 4);
				}
				File.WriteAllBytes(data_path, bytes);
			}
			else
			{
				Debug.LogError("texture format not support:" + path);
			}
		}
		else if (texture is Cubemap)
		{
			var cubemap = (texture as Cubemap);

			if (cubemap.format == TextureFormat.RGB24 || cubemap.format == TextureFormat.RGBA32)
			{
				WriteString("Cubemap");
				m_writer.Write(cubemap.mipmapCount);

				int size = cubemap.width;
				for (int i = 0; i < cubemap.mipmapCount; i++)
				{
					for (int j = 0; j < 6; j++)
					{
						var face_path = string.Format("{0}.cubemap/{1}_{2}.png", path, i, j);

						WriteString(face_path);

						var colors = cubemap.GetPixels((CubemapFace) j, i);
						var face = new Texture2D(size, size, cubemap.format, false);
						for(int k = 0; k < size; k++) {
							Color[] line = new Color[size];
							System.Array.Copy(colors, k * size, line, 0, size);
							face.SetPixels(0, size - k - 1, size, 1, line);
						}

						face_path = m_out_path + "/" + face_path;

						var dir = new FileInfo(face_path).Directory;
						if (dir.Exists == false)
						{
							dir.Create();
						}

						var bytes = face.EncodeToPNG();
						File.WriteAllBytes(face_path, bytes);
					}

					size >>= 1;
				}
			}
			else if (cubemap.format == TextureFormat.RGBAHalf)
			{
				WriteString("CubemapRGBFloat");
				m_writer.Write(cubemap.mipmapCount);

				int size = cubemap.width;
				for (int i = 0; i < cubemap.mipmapCount; i++)
				{
					for (int j = 0; j < 6; j++)
					{
						var face_path = string.Format("{0}.cubemap/{1}_{2}.f", path, i, j);

						WriteString(face_path);

						var colors = cubemap.GetPixels((CubemapFace) j, i);

						face_path = m_out_path + "/" + face_path;

						var dir = new FileInfo(face_path).Directory;
						if (dir.Exists == false)
						{
							dir.Create();
						}

						var bytes = new byte[colors.Length * 12];
						for (int k = 0; k < colors.Length; k++)
						{
							var r = System.BitConverter.GetBytes(colors[k].r);
							var g = System.BitConverter.GetBytes(colors[k].g);
							var b = System.BitConverter.GetBytes(colors[k].b);

							System.Array.Copy(r, 0, bytes, k * 12 + 0, 4);
							System.Array.Copy(g, 0, bytes, k * 12 + 4, 4);
							System.Array.Copy(b, 0, bytes, k * 12 + 8, 4);
						}
						File.WriteAllBytes(face_path, bytes);
					}

					size >>= 1;
				}
			}
			else
			{
				Debug.LogError("texture format not support:" + path);
			}
		}
		else
		{
			Debug.LogError("texture type not support:" + texture.GetType());
		}

		File.WriteAllBytes(file_path.FullName, ms.ToArray());

		m_writer.Close();
		m_writer = save;
	}
}
