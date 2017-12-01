using UnityEditor;
using UnityEngine;
using System.IO;

public partial class Exporter : MeshExporter
{
	const int BONE_MAX = 80;

	[MenuItem("Viry3D/Export/GameObject")]
	public static void ExportGameObjectAll()
	{
		ExportGameObject(false);
	}

	[MenuItem("Viry3D/Export/GameObject (Active Only)")]
	public static void ExportGameObjectActiveOnly()
	{
		ExportGameObject(true);
	}

	[MenuItem("Viry3D/Export/LightMap")]
	static void WriteLightMap()
	{
		var maps = LightmapSettings.lightmaps;

		MemoryStream ms = Init();

		int map_count = maps.Length;
		m_writer.Write(map_count);

		for (int i = 0; i < map_count; i++)
		{
			WriteTexture(maps[i].lightmapColor);
		}

		var scene = UnityEditor.SceneManagement.EditorSceneManager.GetActiveScene();
		var file_path = new FileInfo(m_out_path + "/Assets/" + scene.name + ".lightmap");
		if (!file_path.Directory.Exists)
		{
			Directory.CreateDirectory(file_path.Directory.FullName);
		}
		File.WriteAllBytes(file_path.FullName, ms.ToArray());

		Deinit();

		Log("export lightmap done.");
	}

	static void ExportGameObject(bool active_only)
	{
		var obj = Selection.activeGameObject;
		if (obj == null)
		{
			return;
		}

		MemoryStream ms = Init();

		WriteVersion();

		WriteTransform(obj.transform, active_only);

		var file_path = new FileInfo(m_out_path + "/Assets/" + obj.name + ".prefab");
		if (!file_path.Directory.Exists)
		{
			Directory.CreateDirectory(file_path.Directory.FullName);
		}
		File.WriteAllBytes(file_path.FullName, ms.ToArray());

		Deinit();

		Log("export prefab " + obj.name + " done.");
	}

	static void WriteTransform(Transform t, bool active_only)
	{
		WriteString(t.name);
		m_writer.Write(t.gameObject.layer);
		m_writer.Write(t.gameObject.activeSelf);
		bool is_static = (GameObjectUtility.GetStaticEditorFlags(t.gameObject) & StaticEditorFlags.BatchingStatic) != 0;
		m_writer.Write(is_static);

		WriteVector3(t.localPosition);
		WriteQuaternion(t.localRotation);
		WriteVector3(t.localScale);
		m_writer.Write(t.GetInstanceID());

		int com_count = 0;
		var renderer = t.GetComponent<Renderer>();
		var anim = t.GetComponent<Animation>();
		var particle_system = t.GetComponent<ParticleSystem>();
		var terrain = t.GetComponent<Terrain>();

		bool split_skin = false;

		if (renderer != null && renderer is SkinnedMeshRenderer)
		{
			var skin = renderer as SkinnedMeshRenderer;
			if (skin.bones.Length > BONE_MAX)
			{
				split_skin = true;
				SplitSkinnedMeshRenderer(skin);
			}
		}

		if (renderer != null && split_skin == false)
		{
			com_count++;
		}
		if (anim != null)
		{
			com_count++;
		}
		if (particle_system != null)
		{
			com_count++;
		}
		if (terrain != null)
		{
			com_count++;
		}

		m_writer.Write(com_count);

		if (renderer != null && split_skin == false)
		{
			string type_name = "";

			if (renderer is SkinnedMeshRenderer)
			{
				type_name = "SkinnedMeshRenderer";
			}
			else if (renderer is MeshRenderer)
			{
				type_name = "MeshRenderer";
			}
			else if (renderer is ParticleSystemRenderer)
			{
				type_name = "ParticleSystemRenderer";
			}

			WriteString(type_name);

			switch (type_name)
			{
				case "SkinnedMeshRenderer":
					WriteSkinnedMeshRenderer(renderer as SkinnedMeshRenderer);
					break;
				case "MeshRenderer":
					WriteMeshRenderer(renderer as MeshRenderer);
					break;
				case "ParticleSystemRenderer":
					WriteParticleSystemRenderer(renderer as ParticleSystemRenderer);
					break;
			}
		}

		if (anim != null)
		{
			WriteString("Animation");

			WriteAnimation(anim);
		}

		if (particle_system != null)
		{
			WriteString("ParticleSystem");

			WriteParticleSystem(particle_system);
		}

		if (terrain != null)
		{
			WriteString("Terrain");

			WriteTerrain(terrain);
		}

		int child_count = t.childCount;

		if (active_only)
		{
			int active_count = 0;
			for (int i = 0; i < child_count; i++)
			{
				var child = t.GetChild(i);
				if (child.gameObject.activeInHierarchy)
				{
					active_count++;
				}
			}

			m_writer.Write(active_count);
		}
		else
		{
			m_writer.Write(child_count);
		}

		for (int i = 0; i < child_count; i++)
		{
			var child = t.GetChild(i);

			if (child.gameObject.activeInHierarchy || !active_only)
			{
				WriteTransform(child, active_only);
			}
		}
	}
}
